/**
 * @author Alejandro Solozabal
 *
 * @file liveview.hpp
 *
 */

#ifndef LIVEVIEW_H_
#define LIVEVIEW_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>

#include "common.hpp"
#include "global_parameters.hpp"
#include "log.hpp"
#include "kinect_interface.hpp"
#include "cyclic_task.hpp"
#include "alarm_module_interface.hpp"

/*******************************************************************
 * Struct declaration
 *******************************************************************/
struct LiveviewConfig : AlarmModuleConfig
{
    uint32_t video_frame_interval_ms;

    LiveviewConfig()
    {
    }

    LiveviewConfig(uint32_t _video_frame_interval_ms) : 
        video_frame_interval_ms(_video_frame_interval_ms)
    {
    }
};

/*******************************************************************
 * Class declaration
 *******************************************************************/
class LiveviewObserver
{
public:
    virtual void NewFrame(KinectVideoFrame& frame) = 0;
};

class Liveview : public IAlarmModule, public CyclicTask
{
public:
    /**
     * @brief Construct a new Liveview object
     * 
     */
    Liveview(std::shared_ptr<IKinect> kinect, std::shared_ptr<LiveviewObserver> liveview_observer, LiveviewConfig liveview_config);

    /**
     * @brief Destroy the Liveview object
     * 
     */
    ~Liveview();

    int Start() override;

    int Stop() override;

    bool IsRunning() override;

    void UpdateConfig(AlarmModuleConfig& config) override;

    void ExecutionCycle() override;

private:
    LiveviewConfig m_liveview_config;
    std::shared_ptr<IKinect> m_kinect;
    std::shared_ptr<KinectVideoFrame> m_frame;
    std::shared_ptr<LiveviewObserver> m_liveview_observer;
};

#endif /* LIVEVIEW_H_ */