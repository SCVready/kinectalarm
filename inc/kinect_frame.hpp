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
 * Defines
 *******************************************************************/
#define BLANK_DEPTH_PIXEL 0x07FF

/*******************************************************************
 * Class declaration
 *******************************************************************/
class KinectFrame
{
public:
    KinectFrame(uint32_t width, uint32_t height);
    ~KinectFrame();
    KinectFrame& operator=(const KinectFrame& kinect);

    void Fill(const uint16_t* frame_data);
    const uint16_t* GetDataPointer() const;
    uint32_t ComputeDifferences(KinectFrame& frame, uint32_t tolerance);
    uint32_t m_timestamp;
private:
    uint32_t m_width;
    uint32_t m_height;
    std::vector<uint16_t> m_data;
};

#endif /* KINECT_FRAMES_H_ */
