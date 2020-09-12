/**
 * @author Alejandro Solozabal
 *
 * @file liveview.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "liveview.hpp"
#include "kinect_frame.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/

Liveview::Liveview(std::shared_ptr<Kinect> kinect, uint32_t loop_period_ms) :
    CyclicTask("Liveview", loop_period_ms),
    m_kinect(kinect)
{
    m_frame_ex = std::make_unique<KinectFrame>(VIDEO_WIDTH,VIDEO_HEIGHT);
    m_frame = (uint16_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t));
    m_timestamp = 0;
}

Liveview::~Liveview()
{
    ;
}

void Liveview::ExecutionCycle()
{
    m_kinect->GetVideoFrame(m_frame,&m_timestamp);
    LOG(LOG_INFO,"Liveview cycle: frame taken\n");
}