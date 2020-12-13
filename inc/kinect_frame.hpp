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
    /**
     * @brief Constructor
     *
     *
     * @param[in] width : pixel width of the frame
     * @param[in] height : pixel height of the frame
     */
    KinectFrame(uint32_t width, uint32_t height);

    /**
     * @brief Copy Constructor
     *
     * @param[in] kinect_frame : kinect frame object to be copied
     */
    KinectFrame(const KinectFrame& kinect_frame);

    /**
     * @brief Destructor
     */
    ~KinectFrame();

    /**
     * @brief Operator=
     *
     * @param[in] kinect_frame : kinect frame object to be copied
     */
    KinectFrame& operator=(const KinectFrame& kinect);

    /**
     * @brief Set the content and timestamp of the frame
     *
     * @param[in] frame_data : frame data
     * @param[in] timestamp : timestamp related to the frame
     */
    void Fill(const uint16_t* frame_data, uint32_t timestamp);

    /**
     * @brief Get the pointer to the frame data
     * 
     */
    const uint16_t* GetDataPointer() const;

    /* TODO: its only valid for depth frames */
    /* TODO: its only valid for frames with same dimensions */
    /**
     * @brief Compute differences betwen two depth frames. It done by comparing pixel by pixel the absolute difference
     *        and returning the number of pixels that exceed the tolerance
     *
     * @param[in] frame : frame to compare with
     * @param[in] tolerance : minumum distance value betwen pixel 
     *
     * @return number of pixel that exceeded tolerance
     */
    uint32_t ComputeDifferences(KinectFrame& frame, uint32_t tolerance);

    /**
     * @brief Get the timestamp associated with the frame
     * 
     * @return timestamp associated with the frame
     */
    uint32_t GetTimestamp() const;

    /**
     * @brief Set the timestamp associated with the frame
     * 
     * @param[in] timestamp : timestamp associated with the frame
     */
    void SetTimestamp(uint32_t timestamp);

private:
    uint32_t m_timestamp;
    uint32_t m_width;
    uint32_t m_height;
    std::vector<uint16_t> m_data;
};

#endif /* KINECT_FRAMES_H_ */
