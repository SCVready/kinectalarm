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

#include "cyclic_task.hpp"
#include "kinect_frame.hpp"
#include "common.hpp"
#include "global_parameters.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define MAX_TILT_WAIT 10 /* Seconds to wait until the kinect's tilting is complete */

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Kinect : public CyclicTask
{
public:
    /**
     * @brief Constructor
     * 
     */
    Kinect();

    /**
     * @brief Destructor
     * 
     */
    virtual ~Kinect();

    /**
     * @brief Initialization
     * 
     */
    int Init();

    /**
     * @brief Termination
     * 
     */
    int Term();

    /**
     * @brief Run kinect image capture
     * 
     */
    int Start();

    /**
     * @brief Stop kinect image capture
     * 
     */
    int Stop();

    /**
     * @brief Synchonous function to get a depth frame.
     * 
     * @param[in/out] depth_frame : pointer to an already allocated memory
     *                              array. It will be filled with the frame content
     * @param[in/out] timestamp : to provide the function with the timestamp of the last
     *                            frame you have. It will be updated with the 
     *                            timestamp of the provided frame.
     */
    void GetDepthFrame(std::shared_ptr<KinectFrame> frame);

    /**
     * @brief Synchonous function to get a depth frame.
     * 
     * @param[in/out] video_frame : pointer to an already allocated memory
     *                              array. It will be filled with the frame info
     * @param[in/out] timestamp : to provide te function the timestamp of the last
     *                            frame you have. It will be updated with the 
     *                            timestamp of the provided frame.
     */
    void GetVideoFrame(std::shared_ptr<KinectFrame> frame);

    /**
     * @brief To get change kinect's tilt
     * 
     * @param[in] tilt_angle : wanted kinect's tilt angle, range [-61,61]
     * 
     */
    bool ChangeTilt(double tilt_angle);

    /**
     * @brief To get change kinect's led color
     * 
     * @param[in] color : wanted color
     * 
     * @return 0 on success
     */
    int ChangeLedColor(freenect_led_options color);

private:
    /* Freenect context strucutres */
    freenect_context* m_kinect_ctx;
    freenect_device* m_kinect_dev;

    /* Flags */
    bool m_is_kinect_initialized;

    /* Frames */
    static std::unique_ptr<KinectFrame> m_depth_frame;
    static std::unique_ptr<KinectFrame> m_video_frame;

    /* Concurrency safe */
    static std::mutex m_depth_mutex, m_video_mutex;
    static std::condition_variable m_depth_cv,m_video_cv;

    /* Private funtions */
    static void VideoCallback(freenect_device* dev, void* data, uint32_t timestamp);
    static void DepthCallback(freenect_device* dev, void* data, uint32_t timestamp);
    void ExecutionCycle() override;
};

#endif /* KINECT_H_ */
