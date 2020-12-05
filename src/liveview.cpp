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
Liveview::Liveview(std::shared_ptr<Kinect> kinect, std::shared_ptr<LiveviewObserver> liveview_observer, uint32_t loop_period_ms) :
    CyclicTask("Liveview", loop_period_ms),
    m_kinect(kinect),
    m_liveview_observer(liveview_observer)
{
    m_frame = std::make_unique<KinectFrame>(VIDEO_WIDTH,VIDEO_HEIGHT);
}

Liveview::~Liveview()
{
}

void Liveview::ExecutionCycle()
{
    m_kinect->GetVideoFrame(m_frame);
    LOG(LOG_INFO,"Liveview cycle: frame taken\n");

    m_liveview_observer->NewFrame(m_frame);
}