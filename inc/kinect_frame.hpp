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
#include <vector>

/*******************************************************************
 * Class declaration
 *******************************************************************/
class KinectFrame
{
public:
    KinectFrame();
    ~KinectFrame();
private:
    uint32_t height;
    uint32_t width;
    uint32_t timestamp;
    std::vector<uint8_t> data;
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
