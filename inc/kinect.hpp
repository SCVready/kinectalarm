/**
 * @author Alejandro Solozabal
 *
 * @file kinect.hpp
 *
 */

#ifndef KINECT_H_
#define KINECT_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <memory>
#include <mutex>
#include <condition_variable>
#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

#include "kinect_interface.hpp"
#include "cyclic_task.hpp"
#include "kinect_frame.hpp"
#include "common.hpp"
#include "global_parameters.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Kinect : public IKinect, public CyclicTask
{
public:
    Kinect(uint32_t timeout_ms);
    virtual ~Kinect();

    int Init() override;
    int Term() override;
    int Start() override;
    int Stop() override;
    bool IsRunning() override;
    void GetDepthFrame(KinectDepthFrame& frame) override;
    void GetVideoFrame(KinectVideoFrame& frame) override;
    int ChangeTilt(double tilt_angle) override;
    int ChangeLedColor(freenect_led_options color) override;

private:
    /* Freenect context strucutres */
    freenect_context* m_kinect_ctx;
    freenect_device* m_kinect_dev;

    /* Flags */
    bool m_is_kinect_initialized;

    /* Get frames timeout in ms */
    static uint32_t m_timeout_ms;

    /* Frames */
    static std::unique_ptr<KinectDepthFrame> m_depth_frame;
    static std::unique_ptr<KinectVideoFrame> m_video_frame;

    /* Concurrency safe */
    static std::mutex m_depth_mutex, m_video_mutex;
    static std::condition_variable m_depth_cv,m_video_cv;

    /* Private funtions */
    static void VideoCallback(freenect_device* dev, void* data, uint32_t timestamp);
    static void DepthCallback(freenect_device* dev, void* data, uint32_t timestamp);
    void ExecutionCycle() override;
};

#endif /* KINECT_H_ */
