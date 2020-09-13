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
    KinectFrame(uint32_t width, uint32_t height);
    ~KinectFrame();
    void Fill(uint16_t* frame_data);
    uint16_t* GetDataPointer();
    uint32_t ComputeDifferences(KinectFrame& frame, uint32_t tolerance);
    uint32_t m_timestamp;
private:
    uint32_t m_width;
    uint32_t m_height;
    std::vector<uint16_t> m_data;
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
