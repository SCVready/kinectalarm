/**
 * @author Alejandro Solozabal
 *
 * @file detection.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "detection.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
Detection::Detection(std::shared_ptr<Kinect> kinect, std::shared_ptr<DetectionObserver> detection_observer, uint32_t loop_period_ms) :
    CyclicTask("Detection", loop_period_ms),
    m_current_state(State::Idle),
    m_kinect(kinect),
    m_detection_num(0),
    m_detection_observer(detection_observer)
{
    m_depth_frame_reff        = std::make_unique<KinectFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_depth_frame             = std::make_unique<KinectFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_refresh_reference_frame = std::make_unique<RefreshReferenceFrame>(kinect, m_depth_frame_reff, 1000);
    m_take_video_frames       = std::make_unique<TakeVideoFrames>(*this, kinect, 200);
}

Detection::~Detection()
{
}

void Detection::Start(uint32_t detection_num)
{
    /* Reset intrusion variables */
    m_current_state = State::Idle;

    m_detection_num = detection_num;

    /* Get Reference Depth frame */
    m_kinect->GetDepthFrame(*m_depth_frame_reff);
    LOG(LOG_INFO,"Detection: Depth reference frame\n");

    /* Call parent Start to start the execution thread*/
    CyclicTask::Start();
}

void Detection::ExecutionCycle()
{
    /* Get depth frame */
    m_kinect->GetDepthFrame(*m_depth_frame);

    uint32_t diff = m_depth_frame->ComputeDifferences((*m_depth_frame_reff.get()),10);

    LOG(LOG_DEBUG,"Detection: Diff %d\n", diff);

    bool detected_movement = diff > 2000 ? true : false; 

    switch (m_current_state)
    {
    case State::Idle:
        if(detected_movement)
        {
            m_detection_observer->IntrusionStarted();
            m_take_video_frames->Start(m_detection_num);
            m_refresh_reference_frame->Start();
            m_current_state = State::Intrusion;
            LOG(LOG_WARNING,"Detection: Intrusion started\n");
        }
        break;
    case State::Intrusion:
        if(!detected_movement)
        {
            m_current_state = State::Cooldown;
            m_intrusion_cooldown = std::chrono::system_clock::now() + std::chrono::milliseconds(2000);
        }
        break;
    case State::Cooldown:
        if(detected_movement)
        {
            m_current_state = State::Intrusion;
        }
        else
        {
            if(std::chrono::system_clock::now() > m_intrusion_cooldown)
            {
                uint32_t num_frames = m_take_video_frames->Stop();
                m_refresh_reference_frame->Stop();
                LOG(LOG_WARNING,"Detection: Intrusion Stopped\n");
                m_detection_observer->IntrusionStopped(m_detection_num, num_frames);
                m_detection_num++;
                m_current_state = State::Idle;
            }
        }
        break;
    default:
        break;
    }
}

RefreshReferenceFrame::RefreshReferenceFrame(std::shared_ptr<Kinect> kinect,
                                             std::shared_ptr<KinectFrame> depth_frame_reff,
                                             uint32_t loop_period_ms) :
    CyclicTask("RefreshReferenceFrame", loop_period_ms),
    m_depth_frame_reff(depth_frame_reff),
    m_kinect(kinect)
{
}

void RefreshReferenceFrame::ExecutionCycle()
{
    /*TODO: m_depth_frame_reff atomic*/
    m_kinect->GetDepthFrame(*m_depth_frame_reff);
}

TakeVideoFrames::TakeVideoFrames(Detection& detection,
                                 std::shared_ptr<Kinect> kinect,
                                 uint32_t loop_period_ms) :
    CyclicTask("TakeVideoFrames", loop_period_ms),
    m_detection(detection),
    m_kinect(kinect),
    m_curr_detection_num(0),
    m_frame_counter(0)
{
    m_frame = std::make_unique<KinectFrame>(VIDEO_WIDTH, VIDEO_HEIGHT);
}

void TakeVideoFrames::Start(uint32_t curr_detection_num)
{
    m_curr_detection_num = curr_detection_num;
    m_frame_counter = 0;
    CyclicTask::Start();
}

uint32_t TakeVideoFrames::Stop()
{
    CyclicTask::Stop();
    return m_frame_counter;
}

void TakeVideoFrames::ExecutionCycle()
{
    /*TODO: m_depth_frame_reff atomic*/
    m_kinect->GetVideoFrame(*m_frame);
    LOG(LOG_DEBUG,"TakeVideoFrames cycle: frame taken\n");

    m_detection.m_detection_observer->IntrusionFrame(m_frame, m_curr_detection_num, m_frame_counter++);
}
