/**
 * @author Alejandro Solozabal
 *
 * @file kinect.hpp
 *
 */

#ifndef IKINECT_H_
#define IKINECT_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

#include "kinect_frame.hpp"

/*******************************************************************
 * Class declaration
 *******************************************************************/
class IKinect
{
public:
    /**
     * @brief Destructor
     * 
     */
    virtual ~IKinect() {};

    /**
     * @brief Initialization
     * 
     */
    virtual int Init() = 0;

    /**
     * @brief Termination
     * 
     */
    virtual int Term() = 0;

    /**
     * @brief Run kinect image capture
     * 
     */
    virtual int Start() = 0;

    /**
     * @brief Stop kinect image capture
     * 
     */
    virtual int Stop() = 0;

    /**
     * @brief Check if kinect is runnning
     * 
     * @return true if it is running
     */
    virtual bool IsRunning() = 0;

    /**
     * @brief Synchonous function to get a depth frame.
     * 
     * @param[in/out] frame : reference to a frame object
     */
    virtual void GetDepthFrame(KinectDepthFrame& frame) = 0;

    /**
     * @brief Synchonous function to get a depth frame.
     * 
     * @param[in/out] frame : reference to a frame object
     */
    virtual void GetVideoFrame(KinectVideoFrame& frame) = 0;

    /**
     * @brief To get change kinect's tilt
     * 
     * @param[in] tilt_angle : kinect's tilt angle relative ground , range [-61,61]
     * 
     */
    virtual int ChangeTilt(double tilt_angle) = 0;

    /**
     * @brief To get change kinect's led color
     * 
     * @param[in] color : kinect led color options
     * 
     * @return 0 on success
     */
    virtual int ChangeLedColor(freenect_led_options color) = 0;
};

#endif /* IKINECT_H_ */
