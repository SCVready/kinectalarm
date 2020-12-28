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
Liveview::Liveview(std::shared_ptr<IKinect> kinect, std::shared_ptr<LiveviewObserver> liveview_observer, LiveviewConfig liveview_config) :
    CyclicTask("Liveview", liveview_config.video_frame_interval_ms),
    m_liveview_config(liveview_config),
    m_kinect(kinect),
    m_liveview_observer(liveview_observer)
{
    m_frame = std::make_unique<KinectVideoFrame>(VIDEO_WIDTH,VIDEO_HEIGHT);
}

Liveview::~Liveview()
{
}

void Liveview::UpdateConfig(LiveviewConfig liveview_config)
{
    m_liveview_config = liveview_config;

    CyclicTask::ChangeLoopInterval(liveview_config.video_frame_interval_ms);
}

void Liveview::ExecutionCycle()
{
    m_kinect->GetVideoFrame(*m_frame);
    LOG(LOG_INFO,"Liveview cycle: frame taken\n");

    m_liveview_observer->NewFrame(*m_frame);
}