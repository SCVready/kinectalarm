/**
 * @author Alejandro Solozabal
 *
 * @file detection.hpp
 *
 */

#ifndef DETECTION_H_
#define DETECTION_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>
#include <chrono>

#include "common.hpp"
#include "global_parameters.hpp"
#include "log.hpp"
#include "kinect_interface.hpp"
#include "cyclic_task.hpp"
#include "alarm_module_interface.hpp"

/*******************************************************************
 * Struct declaration
 *******************************************************************/
struct DetectionConfig : AlarmModuleConfig
{
    uint16_t threshold;
    uint16_t sensitivity;
    uint32_t cooldown_ms;
    uint32_t refresh_reference_interval_ms;
    uint32_t take_depth_frame_interval_ms;
    uint32_t take_video_frame_interval_ms;

    DetectionConfig()
    {
    }

    DetectionConfig(uint16_t _threshold,
                    uint16_t _sensitivity,
                    uint32_t _cooldown_ms,
                    uint32_t _refresh_reference_interval_ms,
                    uint32_t _take_depth_frame_interval_ms,
                    uint32_t _take_video_frame_interval_ms) :
                    threshold(_threshold),
                    sensitivity(_sensitivity),
                    cooldown_ms(_cooldown_ms),
                    refresh_reference_interval_ms(_refresh_reference_interval_ms),
                    take_depth_frame_interval_ms(_take_depth_frame_interval_ms),
                    take_video_frame_interval_ms(_take_video_frame_interval_ms)
    {
    }
};

/*******************************************************************
 * Class declaration
 *******************************************************************/
class RefreshReferenceFrame;
class TakeVideoFrames;

class DetectionObserver
{
public:
    virtual void IntrusionStarted() = 0;
    virtual void IntrusionStopped(uint32_t frame_num) = 0;
    virtual void IntrusionFrame(std::shared_ptr<KinectVideoFrame> frame, uint32_t frame_num) = 0;
};

class Detection : public IAlarmModule, public CyclicTask
{
    friend TakeVideoFrames;
public:
    /**
     * @brief Construct a new Detection object
     * 
     */
    Detection(std::shared_ptr<IKinect> kinect, std::shared_ptr<DetectionObserver> detection_observer, DetectionConfig detection_config);

    /**
     * @brief Destroy the Detection object
     * 
     */
    ~Detection();

    int Start() override;

    int Stop() override;

    bool IsRunning() override;

    void UpdateConfig(AlarmModuleConfig& config) override;

    void ExecutionCycle() override;

private:
    enum class State
    {
        Idle,
        Intrusion,
        Cooldown
    };

    DetectionConfig m_detection_config;
    State m_current_state;
    std::chrono::time_point<std::chrono::system_clock> m_cooldown_abs_time;
    std::shared_ptr<KinectDepthFrame> m_depth_frame_ref;
    std::shared_ptr<KinectDepthFrame> m_depth_frame;
    uint32_t m_timestamp;
    std::shared_ptr<IKinect> m_kinect;
    uint8_t* liveview_jpeg;
    std::unique_ptr<RefreshReferenceFrame> m_refresh_reference_frame;
    std::unique_ptr<TakeVideoFrames> m_take_video_frames;
    std::shared_ptr<DetectionObserver> m_detection_observer;
};

class RefreshReferenceFrame : public CyclicTask
{
public:
    RefreshReferenceFrame(std::shared_ptr<IKinect> kinect,
                          std::shared_ptr<KinectDepthFrame> depth_frame_reff,
                          uint32_t loop_period_ms);
    void ExecutionCycle() override;
private:
    std::shared_ptr<KinectDepthFrame> m_depth_frame_ref;
    std::shared_ptr<IKinect> m_kinect;
};

class TakeVideoFrames : public CyclicTask
{
public:
    TakeVideoFrames(Detection& detection,
                    std::shared_ptr<IKinect> kinect,
                    uint32_t loop_period_ms);
    void ExecutionCycle() override;
    void Start();
    uint32_t Stop();
private:
    Detection& m_detection;
    std::shared_ptr<KinectVideoFrame> m_frame;
    std::shared_ptr<IKinect> m_kinect;
    uint32_t m_frame_counter;
};

#endif /* DETECTION_H_ */