/**
 * @author Alejandro Solozabal
 *
 * @file alarm.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include "alarm.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
Alarm::Alarm()
{
    /* Detection config */
    det_conf.is_active      = false;
    det_conf.threshold      = DETECTION_THRESHOLD;
    det_conf.tolerance      = DEPTH_CHANGE_TOLERANCE;
    det_conf.det_num_shots  = NUM_DETECTIONS_FRAMES;
    det_conf.frame_interval = FRAME_INTERVAL_US/1000000;
    det_conf.curr_det_num   = 0;

    /* LiveView config */
    lvw_conf.is_active      = false;
    lvw_conf.tilt           = 0;
    lvw_conf.brightness     = 0;
    lvw_conf.contrast       = 0;

    /* New implementation */
    m_kinect             = std::make_shared<Kinect>();
    m_liveview           = std::make_unique<Liveview>(m_kinect, 100);
    m_detection_observer = std::make_shared<AlarmDetectionObserver>(*this);
    m_detection          = std::make_unique<Detection>(m_kinect, m_detection_observer, 10);
}

Alarm::~Alarm()
{
}

int Alarm::Init()
{
    /* SQLite initialization */
    if(init_sqlite_db())
    {
        LOG(LOG_ERR,"Error: couldn't intialize SQLite\n");
        m_kinect->Term();
        return -1;
    }

    /* Creation of the detection table on SQLite */
    if(create_det_table_sqlite_db())
    {
        LOG(LOG_ERR,"Error: creating detection table on SQLite\n");
        m_kinect->Term();
        return -1;
    }

    /* Creation of the status table on SQLite */
    if(create_status_table_sqlite_db())
    {
        LOG(LOG_ERR,"Error: creating status table on SQLite\n");
        m_kinect->Term();
        return -1;
    }

    /* Parse status */
    if(read_status(&det_conf,&lvw_conf))
    {
        /* Write new status entry on sqlite status table if there is an error
         * parsing the current one */
        write_status(det_conf,lvw_conf);
    }

    /* Redis db initialization */
    if(init_redis_db())
    {
        LOG(LOG_ERR,"Error: couldn't intialize Redis db\n");
        m_kinect->Term();
        return -1;
    }

    /* Initialize redis vars */
    if(InitVarsRedis())
    {
        LOG(LOG_ERR,"Error: couldn't intialize variables in Redis db\n");
        m_kinect->Term();
        return -1;
    }

    /* Kinect initialization */
    if(m_kinect->Init())
    {
        LOG(LOG_ERR,"Error: couldn't intialize Kinect\n");
        m_kinect->Term();
        return -1;
    }

    /* Update kinect led */
    UpdateLed();

    /* Create base directory to save detection images */
    create_dir((char *)DETECTION_PATH);

    /* Apply status */
    if(det_conf.is_active)
        StartDetection();

    if(lvw_conf.is_active)
        StartLiveview();

    /* Adjust kinect's tilt */
    if(m_kinect->ChangeTilt(lvw_conf.tilt))
    {
        LOG(LOG_ERR,"Fallo al cambiar la inclinacion de kinect\n");
        m_kinect->Term();
        return -1;
    }

    return 0;
}

int Alarm::Term()
{
    StopDetection();
    StopLiveview();

    if(m_kinect->IsRunning())
        m_kinect->Stop();
    m_kinect->Term();

    /* Deinit Redis */
    deinit_redis_db();

    /* Deinit SQLite */
    deinit_sqlite_db();

    return 0;
}

int Alarm::StartDetection()
{
    /* Start kinect */
    if(!m_kinect->IsRunning())
        m_kinect->Start();

    if(!m_detection->IsRunning())
    {
        m_detection->Start();

        /* Change status */
        ChangeDetStatus(DET_ACTIVE,true);

        /* Update Redis DB */
        redis_set_int((char *) "det_status", 1);

        /* Update led */
        UpdateLed();

        /* Launch detection thread */
        //pthread_create(&detection_thread, 0, DetectionThreadHelper, this);

        /* Publish event */
        redis_publish("event_info","Detection started");

        LOG(LOG_INFO,"Detection thread running\n");
        return 0;
    }
    return 1;
}

int Alarm::StopDetection()
{
    if(m_detection->IsRunning())
    {
        m_detection->Stop();

        /* Change status */
        ChangeDetStatus(DET_ACTIVE,false);

        /* Update Redis DB */
        redis_set_int((char *) "det_status", 0);

        /* Update led */
        UpdateLed();

        /* Publish event */
        redis_publish("event_info","Detection stopped");

        LOG(LOG_INFO,"Detection thread stopped\n");

        /* Turn off Kinect */
        if(!m_detection->IsRunning() && !m_liveview->IsRunning())
            m_kinect->Stop();

        return 0;
    }
    return 1;
}

int Alarm::StartLiveview()
{
    /* Start kinect */
    if(!m_kinect->IsRunning())
    {
        m_kinect->Start();
    }

    if(!m_liveview->IsRunning())
    {
        m_liveview->Start();

        /* Change status */
        ChangeLvwStatus(LVW_ACTIVE,true);

        /* Update redis db */
        redis_set_int((char *) "lvw_status", 1);

        /* Update led */
        UpdateLed();

        /* Publish event */
        redis_publish("event_info","Liveview started");
    }

    return 0;
}

int Alarm::StopLiveview()
{
    if(m_liveview->IsRunning())
    {
        m_liveview->Stop();

        /* Change status */
        ChangeLvwStatus(LVW_ACTIVE,false);

        /* Update Redis DB */
        redis_set_int((char *) "lvw_status", 0);

        /* Update led */
        UpdateLed();

        /* Publish event */
        redis_publish("event_info","Liveview stopped");

        LOG(LOG_INFO,"Liveview thread stopped\n");

        /* Turn off Kinect */
        if(!m_detection->IsRunning() && !m_liveview->IsRunning())
            m_kinect->Stop();
    }
    return 0;
}

#if 0
/* Liveview stream in h264 */
void *cAlarm::liveview(void)
{
    struct timespec sleep_time = {0,41666666};
    struct timespec wakeup_time;
    struct timespec current_time;
    struct timespec sleep_remain_time = {0,0};
    
    init_video();
    start_video();
    
    clock_gettime(CLOCK_MONOTONIC, &wakeup_time);

    while(liveview_running)
    {
        wakeup_time = timeAdd(wakeup_time, sleep_time);
        
        m_kinect->GetVideoFrame(liveview_frame);
        send_frame(liveview_frame);
        
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        sleep_remain_time = timeSub(wakeup_time,current_time);
        printf("usec to sleep:%d\n",sleep_remain_time.tv_nsec/1000);
        
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
    }

    stop_video();
    deinit_video();
    return 0;
}
#endif

void Alarm::UpdateLed()
{
    if(m_liveview->IsRunning() && m_detection->IsRunning())
        m_kinect->ChangeLedColor((freenect_led_options)4);
    else if(m_detection->IsRunning())
        m_kinect->ChangeLedColor(LED_YELLOW);
    else if(m_liveview->IsRunning())
        m_kinect->ChangeLedColor((freenect_led_options)5);
    else
        m_kinect->ChangeLedColor(LED_OFF);
}

int Alarm::GetNumDetections()
{
    return det_conf.curr_det_num;
}

int Alarm::ResetDetection()
{
    delete_all_entries_det_table_sqlite_db();
    delete_all_files_from_dir(DETECTION_PATH);

    LOG(LOG_INFO,"Deleted all detection entries\n");

    /* Publish events */
    redis_publish("event_success","Deleted all detections");

    return 0;
}

int Alarm::DeleteDetection(int id)
{
    char command[50];
    sprintf(command, "rm -rf %s/%d_*",DETECTION_PATH,id);
    system(command);
    delete_entry_det_table_sqlite_db(id);

    LOG(LOG_INFO,"Delete detection n°%d\n",id);

    return 0;
}

template <typename T>
int Alarm::ChangeDetStatus(enum enumDet_conf conf_name, T value)
{
    switch(conf_name)
    {
        case DET_ACTIVE:
            det_conf.is_active = value;
            break;
        case THRESHOLD:
            det_conf.threshold = value;
            break;
        case TOLERANCE:
            det_conf.tolerance = value;
            break;
        case DET_NUM_SHOTS:
            det_conf.det_num_shots = value;
            break;
        case FRAME_INTERVAL:
            det_conf.frame_interval = value;
            break;
        case CURR_DET_NUM:
            det_conf.curr_det_num = value;
            break;
    }
    write_status(det_conf,lvw_conf);

    return 0;
}

template <typename T>
int Alarm::ChangeLvwStatus(enum enumLvw_conf conf_name, T value)
{
    switch(conf_name)
    {
        case LVW_ACTIVE:
            lvw_conf.is_active = value;
            break;
        case TILT:
            lvw_conf.tilt = value;
            break;
        case BRIGHTNESS:
            lvw_conf.brightness = value;
            break;
        case CONTRAST:
            lvw_conf.contrast = value;
            break;
    }
    write_status(det_conf,lvw_conf);

    return 0;
}

int Alarm::InitVarsRedis()
{
    if(redis_set_int((char *) "det_status", det_conf.is_active))
        return -1;
    if(redis_set_int((char *) "lvw_status", lvw_conf.is_active))
        return -1;
    if(redis_set_int((char *) "det_numdet", det_conf.curr_det_num-1))
        return -1;
    if(redis_set_int((char *) "tilt", lvw_conf.tilt))
        return -1;
    if(redis_set_int((char *) "brightness", lvw_conf.brightness))
        return -1;
    if(redis_set_int((char *) "contrast", lvw_conf.contrast))
        return -1;
    if(redis_set_int((char *) "threshold", det_conf.threshold))
        return -1;
    if(redis_set_int((char *) "sensitivity", det_conf.tolerance))
        return -1;
    return 0;
}

int Alarm::ChangeTilt(double tilt)
{
    m_kinect->ChangeTilt(tilt);
    redis_set_int((char *) "tilt", (int) tilt);
    ChangeLvwStatus(TILT,tilt);
    LOG(LOG_INFO,"Changed Kinect's titl to: %d\n",(int)tilt);

    return 0;
}

int Alarm::ChangeBrightness(int32_t value)
{
    redis_set_int((char *) "brightness", value);
    ChangeLvwStatus(BRIGHTNESS,value);
    LOG(LOG_INFO,"Changed Kinect's brightness to: %d\n",value);

    return 0;
}

int Alarm::ChangeContrast(int32_t value)
{
    redis_set_int((char *) "contrast", value);
    ChangeLvwStatus(CONTRAST,value);
    LOG(LOG_INFO,"Changed Kinect's contrast to: %d\n",value);

    return 0;
}

int Alarm::ChangeThreshold(int32_t value)
{
    redis_set_int((char *) "threshold", value);
    ChangeDetStatus(THRESHOLD,value);
    LOG(LOG_INFO,"Changed Kinect's threshold to: %d\n",value);

    /* Publish events */
    char message[255];
    sprintf(message, "Threshold changed to %d",value);
    redis_publish("event_success",message);

    return 0;
}

int Alarm::ChangeSensitivity(int32_t value)
{
    redis_set_int((char *) "sensitivity", value);
    ChangeDetStatus(TOLERANCE,value);
    LOG(LOG_INFO,"Changed Kinect's sensitivity to: %d\n",value);
    
    /* Publish events */
    char message[255];
    sprintf(message, "Sensitivity changed to %d",value);
    redis_publish("event_success",message);

    return 0;
}

AlarmDetectionObserver::AlarmDetectionObserver(Alarm& alarm) :
    m_alarm(alarm)
{
    ;
}

void AlarmDetectionObserver::IntrusionStarted()
{
    /* Publish events */
    char message[255];
    sprintf(message, "New Intrusion");
    redis_publish("event_error",message);
    redis_publish("email_send_det","");

    /* Update kinect led */
    m_alarm.m_kinect->ChangeLedColor(LED_RED);
}

void AlarmDetectionObserver::IntrusionStopped(uint32_t det_num, uint32_t frame_num)
{
    char filepath_vid[PATH_MAX];
    char filepath[PATH_MAX];
    sprintf(filepath_vid,"%s/%u_%s",DETECTION_PATH,det_num,"capture_vid.mp4");
    sprintf(filepath,"%s/%u_capture.zip",DETECTION_PATH,det_num);

    /* Publish event */
    char message[255];
    sprintf(message, "newdet %u %u %u",det_num,1000,frame_num);
    redis_publish("new_det",message);

    /* Update SQLite db */
    insert_entry_det_table_sqlite_db(det_num,1000,frame_num,filepath,filepath_vid);

    /* Update Redis db */
    redis_set_int((char *) "det_numdet", det_num);

    /* Change Status */
    m_alarm.ChangeDetStatus(CURR_DET_NUM,det_num+1);
}