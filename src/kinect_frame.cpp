/**
 * @author Alejandro Solozabal
 *
 * @file kinect_frame.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <cstdlib>

#include <libfreenect/libfreenect.h>
#include <libfreenect/libfreenect_sync.h>

#include "kinect_frame.hpp"
#include "global_parameters.hpp"
#include "log.hpp"

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
    m_data.assign(frame_data, frame_data + (m_width * m_height));
}

uint16_t* KinectFrame::GetDataPointer()
{
    return m_data.data();
}

uint32_t KinectFrame::ComputeDifferences(KinectFrame& other, uint32_t tolerance)
{
    uint32_t count = 0;

    for(int i = 0; i <(DEPTH_WIDTH*DEPTH_HEIGHT);i++)
    {
        if(m_data[i] != 0x07FF && other.m_data[i] != 0x07FF)
        {
            if((abs(m_data[i] - other.m_data[i])) > tolerance)
            {
                count++;
            }
        }
    }
    return count;
}

#if 0

uint32_t Alarm::CompareDepthFrameToReferenceDepthFrame()
{
    uint32_t cont = 0;

    pthread_mutex_lock(&diff_depth_frame_lock);
    for(int i = 0; i <(DEPTH_WIDTH*DEPTH_HEIGHT);i++)
    {

        if(depth_frame[i] == 0x07FF || reff_depth_frame[i] == 0x07FF)
            diff_depth_frame[i] = 0;
        else
        {
            diff_depth_frame[i] =abs(depth_frame[i] - reff_depth_frame[i]);
            if(diff_depth_frame[i] > det_conf.tolerance)
                cont++;
            else
                diff_depth_frame[i] = 0;
        }
    }
    pthread_mutex_unlock(&diff_depth_frame_lock);
    return cont;
}

#endif