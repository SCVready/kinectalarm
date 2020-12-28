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
#include <string>
#include <vector>
#include <mutex>

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

    /**
     * @brief Save the frame to file in JPEG format.
     *
     * @param[in] path : path for saving the JPEG file
     * @param[in] brightness : brightness adjusts for the JPEG image
     * @param[in] contrast : contrast adjusts for the JPEG image
     *
     * @return number of pixel that exceeded tolerance
     */
    virtual int SaveToJpegInFile(std::string path, int32_t brightness, int32_t contrast) = 0;

    /**
     * @brief Save the frame to memory in JPEG format.
     *
     * @param[in] jpeg_frame : vector object where the JPEG image will be saved
     * @param[in] brightness : brightness adjusts for the JPEG image
     * @param[in] contrast : contrast adjusts for the JPEG image
     *
     * @return number of pixel that exceeded tolerance
     */
    virtual int SaveToJpegInMemory(std::vector<uint8_t>& jpeg_frame, int32_t brightness, int32_t contrast)  = 0;

protected:
    std::mutex m_mutex;
    uint32_t m_timestamp;
    uint32_t m_width;
    uint32_t m_height;
    std::vector<uint16_t> m_data;
};

class KinectDepthFrame : public KinectFrame
{
public:
    KinectDepthFrame(uint32_t width, uint32_t height);
    KinectDepthFrame(const KinectDepthFrame& kinect_depth_frame);
    ~KinectDepthFrame();

    int SaveToJpegInFile(std::string path, int32_t brightness, int32_t contrast) override;
    int SaveToJpegInMemory(std::vector<uint8_t>& jpeg_frame, int32_t brightness, int32_t contrast) override;

    /**
     * @brief Compute differences betwen two depth frames. It done by comparing pixel by pixel the absolute difference
     *        and returning the number of pixels that exceed the tolerance
     *
     * @param[in] frame : frame to compare with
     * @param[in] tolerance : minumum distance value betwen pixel 
     *
     * @return number of pixel that exceeded tolerance
     */
    uint32_t ComputeDifferences(KinectDepthFrame& frame, uint32_t tolerance);
};

class KinectVideoFrame : public KinectFrame
{
public:
    KinectVideoFrame(uint32_t width, uint32_t height);
    KinectVideoFrame(const KinectVideoFrame& kinect_depth_frame);
    ~KinectVideoFrame();

    int SaveToJpegInFile(std::string path, int32_t brightness, int32_t contrast) override;
    int SaveToJpegInMemory(std::vector<uint8_t>& jpeg_frame, int32_t brightness, int32_t contrast) override;
};

#endif /* KINECT_FRAMES_H_ */
