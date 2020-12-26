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
#include "kinect.hpp"
#include "cyclic_task.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class RefreshReferenceFrame;
class TakeVideoFrames;

class DetectionObserver
{
public:
    virtual void IntrusionStarted() = 0;
    virtual void IntrusionStopped(uint32_t det_num, uint32_t frame_num) = 0;
    virtual void IntrusionFrame(std::shared_ptr<KinectVideoFrame> frame, uint32_t det_num, uint32_t frame_num) = 0;
};

class Detection : public CyclicTask
{
    friend TakeVideoFrames;
public:
    /**
     * @brief Construct a new Detection object
     * 
     */
    Detection(std::shared_ptr<Kinect> kinect, std::shared_ptr<DetectionObserver> detection_observer, uint32_t loop_period_ms);

    /**
     * @brief Destroy the Detection object
     * 
     */
    ~Detection();

    void Start(uint32_t detection_num);

    void ExecutionCycle() override;

private:
    enum class State
    {
        Idle,
        Intrusion,
        Cooldown
    };

    State m_current_state;
    std::chrono::time_point<std::chrono::system_clock> m_intrusion_cooldown;
    std::shared_ptr<KinectDepthFrame> m_depth_frame_reff;
    std::shared_ptr<KinectDepthFrame> m_depth_frame;
    uint32_t m_timestamp;
    std::shared_ptr<Kinect> m_kinect;
    uint8_t* liveview_jpeg;
    std::unique_ptr<RefreshReferenceFrame> m_refresh_reference_frame;
    std::unique_ptr<TakeVideoFrames> m_take_video_frames;
    uint32_t m_detection_num;
    std::shared_ptr<DetectionObserver> m_detection_observer;
};

class RefreshReferenceFrame : public CyclicTask
{
public:
    RefreshReferenceFrame(std::shared_ptr<Kinect> kinect,
                          std::shared_ptr<KinectDepthFrame> depth_frame_reff,
                          uint32_t loop_period_ms);
    void ExecutionCycle() override;
private:
    std::shared_ptr<KinectDepthFrame> m_depth_frame_reff;
    std::shared_ptr<Kinect> m_kinect;
};

class TakeVideoFrames : public CyclicTask
{
public:
    TakeVideoFrames(Detection& detection,
                    std::shared_ptr<Kinect> kinect,
                    uint32_t loop_period_ms);
    void ExecutionCycle() override;
    void Start(uint32_t curr_detection_num);
    uint32_t Stop();
private:
    Detection& m_detection;
    std::shared_ptr<KinectVideoFrame> m_frame;
    std::shared_ptr<Kinect> m_kinect;
    uint32_t m_curr_detection_num;
    uint32_t m_frame_counter;
};

#endif /* DETECTION_H_ */