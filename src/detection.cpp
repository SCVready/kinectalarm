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
    m_kinect(kinect),
    m_intrusion(false),
    m_intrusion_cooldown(0),
    m_detection_num(0),
    m_detection_observer(detection_observer)
{
    m_depth_frame_reff        = std::make_unique<KinectFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_depth_frame             = std::make_unique<KinectFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_refresh_reference_frame = std::make_unique<RefreshReferenceFrame>(kinect, m_depth_frame_reff, 1000);
    m_take_video_frames       = std::make_unique<TakeVideoFrames>(kinect, 200);
}

Detection::~Detection()
{
}

void Detection::Start(uint32_t detection_num)
{
    /* Reset intrusion variables */
    m_intrusion = false;
    m_intrusion_cooldown = 0;

    m_detection_num = detection_num;

    /* Get Reference Depth frame */
    m_kinect->GetDepthFrame_ex(m_depth_frame_reff);
    LOG(LOG_INFO,"Detection: Depth reference frame\n");

    /* Call parent Start to start the execution thread*/
    CyclicTask::Start();
}

void Detection::ExecutionCycle()
{
    /* Get depth frame */
    m_kinect->GetDepthFrame_ex(m_depth_frame);

    uint32_t diff = m_depth_frame->ComputeDifferences((*m_depth_frame_reff.get()),10);

    LOG(LOG_DEBUG,"Detection: Diff %d\n", diff);

    if(diff > 2000)
    {
        if(!m_intrusion)
        {
            m_detection_observer->IntrusionStarted();
            m_take_video_frames->Start(m_detection_num);
            m_refresh_reference_frame->Start();
            LOG(LOG_INFO,"Detection: Intrusion started\n");
            m_intrusion = true;
        }
        m_intrusion_cooldown = 0;
    }
    else
    {
        if (m_intrusion)
        {
            if (m_intrusion_cooldown >= 50)
            {
                /* Stop taking video frames. Return: number deteciones*/
                uint32_t num_frames = m_take_video_frames->Stop();
                m_refresh_reference_frame->Stop();
                LOG(LOG_INFO,"Detection: Intrusion Stopped\n");
                m_detection_observer->IntrusionStopped(m_detection_num, num_frames);
                m_intrusion = false;
                m_detection_num++;
            }
            else
            {
                m_intrusion_cooldown++;
                LOG(LOG_DEBUG,"Detection: cooldown %u\n",m_intrusion_cooldown);
            }
        }
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
    m_kinect->GetDepthFrame_ex(m_depth_frame_reff);
}

TakeVideoFrames::TakeVideoFrames(std::shared_ptr<Kinect> kinect,
                               uint32_t loop_period_ms) :
    CyclicTask("TakeVideoFrames", loop_period_ms),
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

#if 0
    /* Package detections */        
    char command[PATH_MAX];
    sprintf(command,"cd %s;zip -q %u_capture.zip %u*",DETECTION_PATH,det_conf.curr_det_num,det_conf.curr_det_num);
    system(command);

#endif
    return m_frame_counter;
}

void TakeVideoFrames::ExecutionCycle()
{
    /*TODO: m_depth_frame_reff atomic*/
    m_kinect->GetVideoFrame_ex(m_frame);
    LOG(LOG_INFO,"TakeVideoFrames cycle: frame taken\n");

    /* Save the frame to JPEG */
    char filepath[PATH_MAX];
    
    sprintf(filepath,"%s/%u_capture_%d.jpeg",DETECTION_PATH, m_curr_detection_num, m_frame_counter++);

    if(save_video_frame_to_jpeg(m_frame->GetDataPointer(),filepath,200,0))
    {
        LOG(LOG_ERR,"Error saving video frame\n");
    }
}
