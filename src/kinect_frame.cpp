/**
 * @author Alejandro Solozabal
 *
 * @file kinect_frame.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <FreeImage.h>

#include "kinect_frame.hpp"
#include "log.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
KinectFrame::KinectFrame(uint32_t width, uint32_t height) :
    m_timestamp(0), m_width(width), m_height(height)
{
    m_data.resize(m_width * m_height);
}

KinectFrame::KinectFrame(const KinectFrame& kinect_frame) :
    m_timestamp(0), m_width(kinect_frame.m_width), m_height(kinect_frame.m_height)
{
    m_data.resize(m_width * m_height);
    *this = kinect_frame;
}

KinectFrame::~KinectFrame()
{
}

KinectFrame& KinectFrame::operator=(const KinectFrame& other)
{
    this->Fill(other.GetDataPointer(), other.GetTimestamp());
    return *this;
}

void KinectFrame::Fill(const uint16_t* frame_data, uint32_t timestamp)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);
    m_data.assign(frame_data, frame_data + (m_width * m_height));
    m_timestamp = timestamp;
}

const uint16_t* KinectFrame::GetDataPointer() const
{
    return m_data.data();
}

uint32_t KinectFrame::GetTimestamp() const
{
    return m_timestamp;
}

void KinectFrame::SetTimestamp(uint32_t timestamp)
{
    m_timestamp = timestamp;
}

KinectDepthFrame::KinectDepthFrame(uint32_t width, uint32_t height) : KinectFrame(width, height)
{
}

KinectDepthFrame::KinectDepthFrame(const KinectDepthFrame& kinect_depth_frame) : KinectFrame(kinect_depth_frame)
{
}

KinectDepthFrame::~KinectDepthFrame()
{
}

uint32_t KinectDepthFrame::ComputeDifferences(KinectDepthFrame& other, uint32_t tolerance)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);
    uint32_t count = 0;

    for(uint32_t i = 0; i < (m_width * m_height); i++)
    {
        if((m_data[i] != BLANK_DEPTH_PIXEL) && (other.m_data[i] != BLANK_DEPTH_PIXEL))
        {
            if((std::abs(static_cast<int32_t>(m_data[i]) - other.m_data[i])) > tolerance)
            {
                count++;
            }
        }
    }
    return count;
}

int KinectDepthFrame::SaveToJpegInFile(std::string path, int32_t brightness, int32_t contrast)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);
    int retval = 0;
    FIBITMAP *depth_bitmap;
    std::vector<uint8_t> bmap(m_width * m_height);

    for(uint32_t i = 0; i <  m_width * m_height; i++)
    {
        bmap[i] = (m_data[i] >> 2);
    }

    depth_bitmap = FreeImage_ConvertFromRawBits(reinterpret_cast<BYTE *>(m_data.data()), m_width, m_height, m_width, 8, 0xFF, 0xFF, 0xFF, true);

    FreeImage_FlipVertical(depth_bitmap);
    FreeImage_AdjustBrightness(depth_bitmap, brightness);
    FreeImage_AdjustContrast(depth_bitmap, contrast);

    if(!FreeImage_Save(FIF_BMP, depth_bitmap, path.c_str(), 0))
    {
        retval = 1;
    }

    FreeImage_Unload(depth_bitmap);

    return retval;
}

int KinectDepthFrame::SaveToJpegInMemory(std::vector<uint8_t>& jpeg_frame, int32_t brightness, int32_t contrast)
{
    /* TODO */
    return 1;
}

KinectVideoFrame::KinectVideoFrame(uint32_t width, uint32_t height) : KinectFrame(width, height)
{
}

KinectVideoFrame::KinectVideoFrame(const KinectVideoFrame& kinect_depth_frame) : KinectFrame(kinect_depth_frame)
{
}

KinectVideoFrame::~KinectVideoFrame()
{
}

int KinectVideoFrame::SaveToJpegInFile(std::string path, int32_t brightness, int32_t contrast)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);
    int retval = 0;
    FIBITMAP *video_bitmap;
    std::vector<uint8_t> bmap(m_width * m_height);

    for(uint32_t i = 0; i <  m_width * m_height; i++)
    {
        bmap[i] = (m_data[i] >> 2);
    }

    video_bitmap = FreeImage_ConvertFromRawBits(reinterpret_cast<BYTE *>(bmap.data()), m_width, m_height, m_width, 8, 0xFF, 0xFF, 0xFF, true);

    FreeImage_AdjustBrightness(video_bitmap, brightness);
    FreeImage_AdjustContrast(video_bitmap, contrast);

    if(!FreeImage_Save(FIF_JPEG, video_bitmap, path.c_str(), 0))
    {
        retval = 1;
    }

    FreeImage_Unload(video_bitmap);

    return retval;
}

int KinectVideoFrame::SaveToJpegInMemory(std::vector<uint8_t>& jpeg_frame, int32_t brightness, int32_t contrast)
{
    std::lock_guard<std::mutex> lock_guard(m_mutex);
    int retval = 0;
    FIBITMAP *video_bitmap;
    FIMEMORY *fi_memory = NULL;
    BYTE *mem_buffer = NULL;
    DWORD size_in_bytes = 0;
    std::vector<uint8_t> bmap(m_width * m_height);

    fi_memory = FreeImage_OpenMemory();

    for(uint32_t i = 0;i <  m_width * m_height; i++)
    {
        bmap[i] = (m_data[i] >> 2);
    }

    video_bitmap = FreeImage_ConvertFromRawBits(reinterpret_cast<BYTE *>(bmap.data()), m_width, m_height, m_width, 8, 0xFF, 0xFF, 0xFF, true);
    FreeImage_AdjustBrightness(video_bitmap, brightness);
    FreeImage_AdjustContrast(video_bitmap, contrast);

    if(!FreeImage_SaveToMemory(FIF_JPEG, video_bitmap, fi_memory, 0))
    {
        retval = true;
    }

    FreeImage_AcquireMemory(fi_memory, &mem_buffer, &size_in_bytes);
    jpeg_frame.assign(mem_buffer, mem_buffer + size_in_bytes);

    FreeImage_CloseMemory(fi_memory);
    FreeImage_Unload(video_bitmap);

    return retval;
}
