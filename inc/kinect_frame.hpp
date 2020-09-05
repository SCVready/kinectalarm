/**
 * @author Alejandro Solozabal
 *
 * @file kinect_frames.hpp
 *
 */

#ifndef KINECT_FRAMES_H_
#define KINECT_FRAMES_H_

/*******************************************************************
 * Includes
 *******************************************************************/


/*******************************************************************
 * Class declaration
 *******************************************************************/
class KinectFrame
{
public:
    ;
private:
    uint32_t height;
    uint32_t width;
    uint32_t timestamp;
};

class VideoFrame : public KinectFrame
{
public:
    ;
private:
    ;
};

class DepthFrame : public KinectFrame
{
public:
    ;
private:
    ;
};

#endif /* KINECT_FRAMES_H_ */
