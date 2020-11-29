/**
 * @author Alejandro Solozabal
 *
 * @file kinect_frame.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <cmath>
#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

#include "kinect_frame.hpp"
#include "global_parameters.hpp"
#include "log.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

KinectFrame::KinectFrame(uint32_t width, uint32_t height) :
    m_timestamp(0), m_width(width), m_height(height)
{
    m_data.resize(width * height);
}

KinectFrame::~KinectFrame()
{
    ;
}

KinectFrame& KinectFrame::operator=(const KinectFrame& other)
{
    this->Fill(other.GetDataPointer());
    this->m_timestamp = other.m_timestamp;
    return *this;
}

void KinectFrame::Fill(const uint16_t* frame_data)
{
    m_data.assign(frame_data, frame_data + (m_width * m_height));
}

const uint16_t*  KinectFrame::GetDataPointer() const
{
    return m_data.data();
}

uint32_t KinectFrame::ComputeDifferences(KinectFrame& other, uint32_t tolerance)
{
    uint32_t count = 0;

    for(int i = 0; i < (DEPTH_WIDTH * DEPTH_HEIGHT); i++)
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
