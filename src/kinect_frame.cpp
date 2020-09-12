/**
 * @author Alejandro Solozabal
 *
 * @file kinect_frame.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

#include "kinect_frame.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

KinectFrame::KinectFrame(uint32_t width, uint32_t height) :
    m_width(width), m_height(height)
{
    m_data.resize(width * height);
}

KinectFrame::~KinectFrame()
{
    ;
}

void KinectFrame::Fill(uint16_t* frame_data)
{
    m_data.insert(m_data.begin(), frame_data, frame_data + (m_width * m_height));
}