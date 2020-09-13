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
    m_num_detection(0),
    m_detection_observer(detection_observer)
{
    m_depth_frame_reff = std::make_unique<KinectFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_depth_frame = std::make_unique<KinectFrame>(DEPTH_WIDTH,DEPTH_HEIGHT);
    m_refresh_reference_frame = std::make_unique<RefreshReferenceFrame>(kinect, m_depth_frame_reff, 1000);
    m_take_video_frames = std::make_unique<TakeVideoFrames>(kinect, 200);
}

Detection::~Detection()
{
}

void Detection::Start()
{
    m_intrusion_cooldown = 0;
    m_intrusion = false;

    /* Get reference depth frame */
    m_kinect->GetDepthFrame_ex(m_depth_frame_reff);
    LOG(LOG_INFO,"Detection: Depth reference frame\n");

    CyclicTask::Start();
}

void Detection::ExecutionCycle()
{
    /* Get reference depth frame */

    m_kinect->GetDepthFrame_ex(m_depth_frame);

    uint32_t diff = m_depth_frame->ComputeDifferences((*m_depth_frame_reff.get()),10);

    LOG(LOG_DEBUG,"Detection: Diff %d\n", diff);

    if(diff > 1000)
    {
        if(!m_intrusion)
        {
            m_detection_observer->IntrusionStarted();
            m_take_video_frames->Start(m_num_detection);
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
                m_detection_observer->IntrusionStopped(m_num_detection, num_frames);
                m_intrusion = false;
                m_num_detection++;
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

#if 0

/* Variable initialization */
    reff_depth_timestamp = 0;
    depth_timestamp      = 0;
    video_timestamp      = 0;
    time_t t;
    int frame_counter = 0;
    struct timespec sleep_time = {0,200000000}; /* TODO: create a parameter (fps) */
    struct timespec wakeup_time;
    struct timespec current_time;
    struct timespec sleep_remain_time = {0,0};

    char message[255];

    while(detection_running)
    {
        /* Update led */
        UpdateLed();

        /* Get Reference frame */
        if(m_kinect->GetDepthFrame(reff_depth_frame,&reff_depth_timestamp))
        {
            LOG(LOG_ERR,"Failed to capture depth frame\n");
            m_kinect->Term();
            return 0;
        }

        /* Detect depth changes */
        int diff_cont = 0;
        do
        {
            /* get depth image to compare */
            if(m_kinect->GetDepthFrame(depth_frame,&depth_timestamp))
            {
                LOG(LOG_ERR,"Failed to capture depth frame\n");
                m_kinect->Term();
                return 0;
            }

            diff_cont = CompareDepthFrameToReferenceDepthFrame();

        }while(diff_cont < det_conf.threshold && detection_running);


        /* Detection occurs */
        if(detection_running)
        {
            /* Get timestamp of the detection */
            time(&t);

            LOG(LOG_ALERT,"Intrusion occurs with %u differences\n",diff_cont);

            /* Publish events */
            sprintf(message, "New Intrusion");
            redis_publish("event_error",message);
            redis_publish("email_send_det","");

            /* Update kinect led */
            m_kinect->ChangeLedColor(LED_RED);

            frame_counter = 0;

            /* Loop getting images until the detection ceases */
            do
            {
                /* Copy last depht frame to the refference frame */
                reff_depth_timestamp = depth_timestamp;
                memcpy(reff_depth_frame,depth_frame,DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));

                clock_gettime(CLOCK_MONOTONIC, &wakeup_time);

                /* Loop getting a fix nunber of frames */
                for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
                {
                    wakeup_time = timeAdd(wakeup_time, sleep_time);

                    /* Getting video frame */
                    if(m_kinect->GetVideoFrame(video_frames[0],&video_timestamp))
                    {
                        LOG(LOG_ERR,"Failed to capture video frame\n");
                        m_kinect->Term();
                        return 0;
                    }

                    /* Save the frame to JPEG */
                    char filepath[PATH_MAX];
                    sprintf(filepath,"%s/%u_capture_%d.jpeg",DETECTION_PATH,det_conf.curr_det_num,frame_counter++);
                    if(save_video_frame_to_jpeg(video_frames[0],filepath,lvw_conf.brightness,lvw_conf.contrast))
                        LOG(LOG_ERR,"Error saving video frame\n");

                    clock_gettime(CLOCK_MONOTONIC, &current_time);
                    sleep_remain_time = timeSub(wakeup_time,current_time);

                    /* Sleep until next frame */
                    if(i != (NUM_DETECTIONS_FRAMES-1))
                        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
                }
                /* Get depth image to compare */
                if(m_kinect->GetDepthFrame(depth_frame,&depth_timestamp))
                {
                    LOG(LOG_ERR,"Failed to capture depth frame\n");
                    m_kinect->Term();
                    return 0;
                }
                diff_cont = CompareDepthFrameToReferenceDepthFrame();
            }while(diff_cont > det_conf.threshold && detection_running);


            char filepath_vid[PATH_MAX];
            char filepath[PATH_MAX];
            sprintf(filepath_vid,"%s/%u_%s",DETECTION_PATH,det_conf.curr_det_num,"capture_vid.mp4");
            sprintf(filepath,"%s/%u_capture.zip",DETECTION_PATH,det_conf.curr_det_num);
#if 0
            /* Save video */
            encode_video_from_frames(filepath_vid,video_frames,5,VIDEO_HEIGHT,VIDEO_WIDTH,5);

            /* Save gif */
            save_video_frames_to_gif(video_frames,NUM_DETECTIONS_FRAMES,(1/0.2),(char *)"asd");
#endif
            /* Package detections */
            char command[PATH_MAX];
            sprintf(command,"cd %s;zip -q %u_capture.zip %u*",DETECTION_PATH,det_conf.curr_det_num,det_conf.curr_det_num);
            system(command);

            /* Publish event */
            sprintf(message, "newdet %u %lu %u",det_conf.curr_det_num,t,frame_counter);
            redis_publish("new_det",message);

            /* Update SQLite db */
            insert_entry_det_table_sqlite_db(det_conf.curr_det_num,t,frame_counter,filepath,filepath_vid); /* TODO: check if it fails */

            /* Update Redis db */
            redis_set_int((char *) "det_numdet", det_conf.curr_det_num); /* TODO: check if it fails */

            /* Change Status */
            ChangeDetStatus(CURR_DET_NUM,det_conf.curr_det_num+1);
        }
    }
    return 0;

#endif