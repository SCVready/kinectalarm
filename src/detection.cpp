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
Detection::Detection(std::shared_ptr<IKinect> kinect, std::shared_ptr<DetectionObserver> detection_observer, DetectionConfig detection_config) :
    CyclicTask("Detection", detection_config.take_depth_frame_interval_ms),
    m_detection_config(detection_config),
    m_current_state(State::Idle),
    m_kinect(kinect),
    m_detection_observer(detection_observer)
{
    m_depth_frame_ref         = std::make_unique<KinectDepthFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_depth_frame             = std::make_unique<KinectDepthFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_refresh_reference_frame = std::make_unique<RefreshReferenceFrame>(kinect, m_depth_frame_ref, detection_config.refresh_reference_interval_ms);
    m_take_video_frames       = std::make_unique<TakeVideoFrames>(*this, kinect, detection_config.take_video_frame_interval_ms);
}

Detection::~Detection()
{
}

int Detection::Start()
{
    int retval = -1;

    /* Reset intrusion variables */
    m_current_state = State::Idle;

    /* Get Reference Depth frame */
    m_kinect->GetDepthFrame(*m_depth_frame_ref);
    LOG(LOG_INFO,"Detection: Depth reference frame\n");

    if(0 != CyclicTask::Start())
    {
        LOG(LOG_ERR,"CyclicTask::Start() failed\n");
    }
    else
    {
        LOG(LOG_INFO,"Detection started successfully\n");
        retval = 0;
    }

    return retval;
}

int Detection::Stop()
{
    int retval = 0;

    m_take_video_frames->Stop();
    m_refresh_reference_frame->Stop();

    /* Call parent Stop to stop the execution thread*/
    if(0 != CyclicTask::Stop())
    {
        LOG(LOG_ERR,"CyclicTask::Stop() failed\n");
        retval = -1;
    }
    else
    {
        LOG(LOG_INFO,"Detection stopped successfully\n");
    }

    return retval;
}

bool Detection::IsRunning()
{
    return CyclicTask::IsRunning();
}

void Detection::UpdateConfig(AlarmModuleConfig& config)
{
    m_detection_config = dynamic_cast<DetectionConfig&>(config);

    CyclicTask::ChangeLoopInterval(m_detection_config.take_depth_frame_interval_ms);
    m_refresh_reference_frame->ChangeLoopInterval(m_detection_config.refresh_reference_interval_ms);
    m_take_video_frames->ChangeLoopInterval(m_detection_config.take_video_frame_interval_ms);
}

void Detection::ExecutionCycle()
{
    /* Get depth frame */
    m_kinect->GetDepthFrame(*m_depth_frame);

    uint32_t diff = m_depth_frame->ComputeDifferences((*m_depth_frame_ref.get()), m_detection_config.sensitivity);

    LOG(LOG_DEBUG,"Detection: Diff %d\n", diff);

    bool detected_movement = diff > m_detection_config.threshold ? true : false; 

    switch (m_current_state)
    {
    case State::Idle:
        if(detected_movement)
        {
            m_detection_observer->IntrusionStarted();
            m_take_video_frames->Start();
            m_refresh_reference_frame->Start();
            m_current_state = State::Intrusion;
            LOG(LOG_WARNING,"Detection: Intrusion started\n");
        }
        break;
    case State::Intrusion:
        if(!detected_movement)
        {
            m_current_state = State::Cooldown;
            m_cooldown_abs_time = std::chrono::system_clock::now() + std::chrono::milliseconds(m_detection_config.cooldown_ms);
        }
        break;
    case State::Cooldown:
        if(detected_movement)
        {
            m_current_state = State::Intrusion;
        }
        else
        {
            if(std::chrono::system_clock::now() > m_cooldown_abs_time)
            {
                uint32_t num_frames = m_take_video_frames->Stop();
                m_refresh_reference_frame->Stop();
                LOG(LOG_WARNING,"Detection: Intrusion Stopped\n");
                m_detection_observer->IntrusionStopped(num_frames);
                m_current_state = State::Idle;
            }
        }
        break;
    default:
        break;
    }
}

RefreshReferenceFrame::RefreshReferenceFrame(std::shared_ptr<IKinect> kinect,
                                             std::shared_ptr<KinectDepthFrame> depth_frame_reff,
                                             uint32_t loop_period_ms) :
    CyclicTask("RefreshReferenceFrame", loop_period_ms),
    m_depth_frame_ref(depth_frame_reff),
    m_kinect(kinect)
{
}

void RefreshReferenceFrame::ExecutionCycle()
{
    m_kinect->GetDepthFrame(*m_depth_frame_ref);
}

TakeVideoFrames::TakeVideoFrames(Detection& detection,
                                 std::shared_ptr<IKinect> kinect,
                                 uint32_t loop_period_ms) :
    CyclicTask("TakeVideoFrames", loop_period_ms),
    m_detection(detection),
    m_kinect(kinect),
    m_frame_counter(0)
{
    m_frame = std::make_unique<KinectVideoFrame>(VIDEO_WIDTH, VIDEO_HEIGHT);
}

void TakeVideoFrames::Start()
{
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
    m_kinect->GetVideoFrame(*m_frame);
    LOG(LOG_DEBUG,"TakeVideoFrames cycle: frame taken\n");

    m_detection.m_detection_observer->IntrusionFrame(m_frame, m_frame_counter++);
}
