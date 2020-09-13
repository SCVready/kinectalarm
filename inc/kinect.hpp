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
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#include <memory>

#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

#include "kinect_frame.hpp"
#include "common.hpp"
#include "global_parameters.hpp"
#include "log.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define MAX_TILT_WAIT 10 /* Seconds to wait until the kinect's tilting is complete */

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Kinect
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
     * @brief Syncronous funtion to get a depth frame.
     * 
     * @param[in/out] depth_frame : pointer to an already allocated memory
     *                              array. It will be filled with the frame content
     * @param[in/out] timestamp : to provide the funtion with the timestamp of the last
     *                            frame you have. It will be updated with the 
     *                            timestamp of the provided frame.
     */
    int GetDepthFrame(uint16_t *depth_frame, uint32_t *timestamp);
    void GetDepthFrame_ex(std::shared_ptr<KinectFrame> frame);

    /**
     * @brief Syncronous funtion to get a depth frame.
     * 
     * @param[in/out] video_frame : pointer to an already allocated memory
     *                              array. It will be filled with the frame info
     * @param[in/out] timestamp : to provide te funtion the timestamp of the last
     *                            frame you have. It will be updated with the 
     *                            timestamp of the provided frame.
     */
    int GetVideoFrame(uint16_t *video_frame, uint32_t *timestamp);
    void GetVideoFrame_ex(std::shared_ptr<KinectFrame> frame);

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
     */
    void ChangeLedColor(freenect_led_options color);

    /**
     * @brief Check if kinect is running
     * 
     */
    bool IsRunning();

private:
    /* Freenect context strucutres */
    freenect_context* kinect_ctx;
    freenect_device* kinect_dev;

    /* Thread */
    pthread_t process_event_thread;

    /* Flags */
    bool is_kinect_initialize;
    volatile bool running;

    /* Frame pointers */
    static uint16_t* temp_depth_frame_raw;
    static uint16_t* temp_video_frame_raw;

    /* Frame timestamp */
    static uint32_t temp_depth_frame_timestamp;
    static uint32_t temp_video_frame_timestamp;

    /* Thread sinc */
    static pthread_mutex_t depth_lock;
    static pthread_mutex_t video_lock;
    static pthread_cond_t depth_ready;
    static pthread_cond_t video_ready;

    static void VideoCallback(freenect_device* dev, void* data, uint32_t timestamp);
    static void DepthCallback(freenect_device* dev, void* data, uint32_t timestamp);
    void *KinectProcessEvents(void);
    static void *KinectProcessEventsHelper(void *context);
};

#endif /* KINECT_H_ */
