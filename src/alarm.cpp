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
#include "kinect_factory.hpp"
#include "alarm_module_factory.hpp"
#include "message_broker_factory.hpp"
#include "state_persistence_factory.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define KINECT_GETFRAMES_TIMEOUT_MS 1000
#define NEW_STATEPERSISTENCE

/*******************************************************************
 * Class definition
 *******************************************************************/
Alarm::Alarm(std::shared_ptr<IMessageBroker> message_broker, std::shared_ptr<IDatabase> data_base) :
    m_message_broker(message_broker),
    m_data_base(data_base)
{
    m_detection_config.threshold = 2000;
    m_detection_config.sensitivity = 10;
    m_detection_config.cooldown_ms = 2000;
    m_detection_config.refresh_reference_interval_ms = 1000;
    m_detection_config.take_depth_frame_interval_ms = 10;
    m_detection_config.take_video_frame_interval_ms = 200;

    m_liveview_config.video_frame_interval_ms = 100;

    m_detection_observer = std::make_shared<AlarmDetectionObserver>(*this);
    m_liveview_observer  = std::make_shared<AlarmLiveviewObserver>(*this);

    m_kinect    = KinectFactory::Create(KINECT_GETFRAMES_TIMEOUT_MS);
    m_detection = AlarmModuleFactory::CreateDetectionModule(m_kinect, m_detection_observer, m_detection_config);
    m_liveview  = AlarmModuleFactory::CreateLiveviewModule(m_kinect, m_liveview_observer, m_liveview_config);
}

Alarm::~Alarm()
{
}

int Alarm::Init()
{
    init_base64encode(&m_c);

    if(InitStatePersistenceVars())
    {
        LOG(LOG_ERR,"Error: couldn't initialize StatePersistence vars\n");
        m_kinect->Term();
        return -1;
    }

    /* Initialize redis vars */
    if(InitVarsRedis())
    {
        LOG(LOG_ERR,"Error: couldn't initialize variables in Redis db\n");
        m_kinect->Term();
        return -1;
    }

    /* Kinect initialization */
    if(m_kinect->Init())
    {
        LOG(LOG_ERR,"Error: couldn't initialize Kinect\n");
        m_kinect->Term();
        return -1;
    }

    /* Update kinect led */
    UpdateLed();

    /* Create base directory to save detection images */ /*TODO move to Main*/
    create_dir((char *)DETECTION_PATH);

    /* Apply status */
    if(m_alarm_config.detection_active)
        StartDetection();

    if(m_alarm_config.liveview_active)
        StartLiveview();

    /* Adjust kinect's tilt */
    if(m_kinect->ChangeTilt(m_alarm_config.tilt))
    {
        LOG(LOG_ERR,"Error: couldn't change Kinect's tilt\n");
        m_kinect->Term();
        return -1;
    }

    return 0;
}

int Alarm::Term()
{
    m_detection->Stop();
    m_liveview->Stop();

    if(m_kinect->IsRunning())
        m_kinect->Stop();
    m_kinect->Term();

    deinit_base64encode(&m_c);

    return 0;
}

int Alarm::StartDetection()
{
    /* Start kinect */
    if(!m_kinect->IsRunning())
        m_kinect->Start();

    if(!m_detection->IsRunning())
    {
        m_alarm_config.detection_active = 1;
        WriteStatus();

        /* Update Redis DB */
        m_message_broker->SetVariable({"det_status",  DataType::Integer, 1});

        m_detection->Start();

        /* Update led */
        UpdateLed();

        /* Publish event */
        m_message_broker->Publish("event_info", "Detection started");

        LOG(LOG_INFO,"Detection thread running\n");
        return 0;
    }
    return 1;
}

int Alarm::StopDetection()
{
    if(m_detection->IsRunning())
    {
        /* Change status */
        m_alarm_config.detection_active = 0;
        WriteStatus();

        /* Update Redis DB */
        m_message_broker->SetVariable({"det_status",  DataType::Integer, 0});

        m_detection->Stop();

        /* Update led */
        UpdateLed();

        /* Publish event */
        m_message_broker->Publish("event_info", "Detection stopped");

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
        /* Change status */
        m_alarm_config.liveview_active = 1;
        WriteStatus();

        /* Update redis db */
        m_message_broker->SetVariable({"lvw_status",  DataType::Integer, 1});

        m_liveview->Start();

        /* Update led */
        UpdateLed();

        /* Publish event */
        m_message_broker->Publish("event_info","Liveview started");
    }

    return 0;
}

int Alarm::StopLiveview()
{
    if(m_liveview->IsRunning())
    {
        /* Change status */
        m_alarm_config.liveview_active = 0;
        WriteStatus();

        /* Update Redis DB */
        m_message_broker->SetVariable({"lvw_status",  DataType::Integer, 0});

        m_liveview->Stop();

        /* Update led */
        UpdateLed();

        /* Publish event */
        m_message_broker->Publish("event_info","Liveview stopped");

        LOG(LOG_INFO,"Liveview thread stopped\n");

        /* Turn off Kinect */
        if(!m_detection->IsRunning() && !m_liveview->IsRunning())
            m_kinect->Stop();
    }
    return 0;
}

void Alarm::UpdateLed()
{
    if(m_detection->IsRunning())
        m_kinect->ChangeLedColor(LED_YELLOW);
    else if(m_liveview->IsRunning())
        m_kinect->ChangeLedColor(LED_GREEN);
    else
        m_kinect->ChangeLedColor(LED_OFF);
}

int Alarm::GetNumDetections()
{
    return m_alarm_config.current_detection_number;
}

int Alarm::ResetDetection()
{
    m_detection_table->DeleteAllItems();
    delete_all_files_from_dir(DETECTION_PATH);

    LOG(LOG_INFO,"Deleted all detection entries\n");

    /* Publish events */
    m_message_broker->Publish("event_success","Deleted all detections");

    return 0;
}

int Alarm::DeleteDetection(int id)
{
    char command[50];
    sprintf(command, "rm -rf %s/%d_*",DETECTION_PATH,id);
    system(command);

    Entry delete_entry = m_detection_table_definition;
    delete_entry[0].value = id; /*ID*/
    m_detection_table->DeleteItem(delete_entry);

    LOG(LOG_INFO,"Delete detection n°%d\n",id);

    return 0;
}

int Alarm::InitVarsRedis()
{
    int rel_val = 0;
    std::array<Variable, 8> variables{{
        {"det_status",  DataType::Integer, m_alarm_config.detection_active},
        {"lvw_status",  DataType::Integer, m_alarm_config.liveview_active},
        {"det_numdet",  DataType::Integer, m_alarm_config.current_detection_number-1},
        {"tilt",        DataType::Integer, m_alarm_config.tilt},
        {"brightness",  DataType::Integer, m_alarm_config.brightness},
        {"contrast",    DataType::Integer, m_alarm_config.contrast},
        {"threshold",   DataType::Integer, m_detection_config.threshold},
        {"sensitivity", DataType::Integer, m_detection_config.sensitivity}
    }};

    for(const auto& variable : variables)
    {
        if(0 != m_message_broker->SetVariable(variable))
        {
            rel_val = -1;
            break;
        }
    }
    return rel_val;
}

int Alarm::InitStatePersistenceVars()
{
    int ret_val = -1;

    if(nullptr == (m_detection_table = StatePersistenceFactory::CreateDatatable(m_data_base, "DETECTIONS", m_detection_table_definition)))
    {
        LOG(LOG_ERR,"CreateDatatable DETECTIONS error\n");
    }
    else if (nullptr == (m_status_table = StatePersistenceFactory::CreateDatatable(m_data_base, "STATUS", m_status_table_definition)))
    {
        LOG(LOG_ERR,"CreateDatatable STATUS error\n");
    }
    else
    {
        if(0 != ReadStatus())
        {
            LOG(LOG_WARNING,"No status entry\n");
            if(0 != CreateStatus())
            {
                ret_val = -1;
            }
            else
            {
                ret_val = 0;
            }
        }
        else
        {
            LOG(LOG_INFO,"Status read\n");
            ret_val = 0;
        }
    }

    return ret_val;
}

int Alarm::ReadStatus()
{
    int ret_val = -1;
    Entry status = m_status_table_definition;
    status[0].value = 0; /*index*/

    if(0 != m_status_table->GetItem(status))
    {
        LOG(LOG_WARNING,"Error reading status table\n");
    }
    else
    {
        m_alarm_config.tilt                              = std::get<int>(status[1].value);
        m_alarm_config.brightness                        = std::get<int>(status[2].value);
        m_alarm_config.contrast                          = std::get<int>(status[3].value);
        m_alarm_config.detection_active                  = std::get<int>(status[4].value);
        m_alarm_config.liveview_active                   = std::get<int>(status[5].value);
        m_alarm_config.current_detection_number          = std::get<int>(status[6].value);
        m_detection_config.threshold                     = std::get<int>(status[7].value);
        m_detection_config.sensitivity                   = std::get<int>(status[8].value);
        m_detection_config.cooldown_ms                   = std::get<int>(status[9].value);
        m_detection_config.refresh_reference_interval_ms = std::get<int>(status[10].value);
        m_detection_config.take_depth_frame_interval_ms  = std::get<int>(status[11].value);
        m_detection_config.take_video_frame_interval_ms  = std::get<int>(status[12].value);
        m_liveview_config.video_frame_interval_ms        = std::get<int>(status[13].value);

        LOG(LOG_INFO,"Status read\n");
        ret_val = 0;
    }

    return ret_val;
}

int Alarm::WriteStatus()
{
    int ret_val = -1;
    Entry status = m_status_table_definition;
    status[0].value = 0; /*index*/

    status[1].value = static_cast<int32_t>(m_alarm_config.tilt); /*TILT*/
    status[2].value = static_cast<int32_t>(m_alarm_config.brightness); /*BRIGHTNESS*/
    status[3].value = static_cast<int32_t>(m_alarm_config.contrast); /*CONTRAST*/
    status[4].value = static_cast<int32_t>(m_alarm_config.detection_active); /*DET_ACTIVE*/
    status[5].value = static_cast<int32_t>(m_alarm_config.liveview_active); /*LVW_ACTIVE*/
    status[6].value = static_cast<int32_t>(m_alarm_config.current_detection_number); /*CURRENT_DET_NUM*/
    status[7].value = static_cast<int32_t>(m_detection_config.threshold); /*DET_THRESHOLD*/
    status[8].value = static_cast<int32_t>(m_detection_config.sensitivity); /*DET_SENSITIVITY*/
    status[9].value = static_cast<int32_t>(m_detection_config.cooldown_ms); /*DET_COOLDOWN_MS*/
    status[10].value = static_cast<int32_t>(m_detection_config.refresh_reference_interval_ms); /*DET_REFRESH_REFERENCE_INTERVAL_MS*/
    status[11].value = static_cast<int32_t>(m_detection_config.take_depth_frame_interval_ms); /*DET_TAKE_DEPTH_FRAME_INTERVAL_MS*/
    status[12].value = static_cast<int32_t>(m_detection_config.take_video_frame_interval_ms); /*DET_TAKE_VIDEO_FRAME_INTERVAL_MS*/
    status[13].value = static_cast<int32_t>(m_liveview_config.video_frame_interval_ms); /*LVW_VIDEO_FRAME_INTERVAL_MS*/

    if(0 != m_status_table->SetItem(status))
    {
        LOG(LOG_WARNING,"Error writing status table\n");
    }
    else
    {
        LOG(LOG_INFO,"Status written\n");
        ret_val = 0;
    }

    return ret_val;
}

int Alarm::CreateStatus()
{
    int ret_val = -1;
    Entry status = m_status_table_definition;
    status[0].value = 0; /*index*/

    status[1].value = 0; /*TILT*/
    status[2].value = 750; /*BRIGHTNESS*/
    status[3].value = 0; /*CONTRAST*/
    status[4].value = 0; /*DET_ACTIVE*/
    status[5].value = 0; /*LVW_ACTIVE*/
    status[6].value = 0; /*CURRENT_DET_NUM*/
    status[7].value = static_cast<int32_t>(m_detection_config.threshold); /*DET_THRESHOLD*/
    status[8].value = static_cast<int32_t>(m_detection_config.sensitivity); /*DET_SENSITIVITY*/
    status[9].value = static_cast<int32_t>(m_detection_config.cooldown_ms); /*DET_COOLDOWN_MS*/
    status[10].value = static_cast<int32_t>(m_detection_config.refresh_reference_interval_ms); /*DET_REFRESH_REFERENCE_INTERVAL_MS*/
    status[11].value = static_cast<int32_t>(m_detection_config.take_depth_frame_interval_ms); /*DET_TAKE_DEPTH_FRAME_INTERVAL_MS*/
    status[12].value = static_cast<int32_t>(m_detection_config.take_video_frame_interval_ms); /*DET_TAKE_VIDEO_FRAME_INTERVAL_MS*/
    status[13].value = static_cast<int32_t>(m_liveview_config.video_frame_interval_ms); /*LVW_VIDEO_FRAME_INTERVAL_MS*/

    if(0 != m_status_table->InsertItem(status))
    {
        LOG(LOG_WARNING,"Error creating status table\n");
    }
    else
    {
        LOG(LOG_INFO,"Status created\n");
        ret_val = 0;
    }

    return ret_val;
}

int Alarm::ChangeTilt(double value)
{
    m_kinect->ChangeTilt(value);
    m_message_broker->SetVariable({"tilt",  DataType::Integer, static_cast<int32_t>(value)});
    m_alarm_config.tilt = value;
    WriteStatus();
    LOG(LOG_INFO,"Changed Kinect's tilt to: %d\n",(int)value);

    return 0;
}

int Alarm::ChangeBrightness(int32_t value)
{
    m_message_broker->SetVariable({"brightness",  DataType::Integer, static_cast<int32_t>(value)});
    m_alarm_config.brightness = value;
    WriteStatus();
    LOG(LOG_INFO,"Changed Kinect's brightness to: %d\n",value);

    return 0;
}

int Alarm::ChangeContrast(int32_t value)
{
    m_message_broker->SetVariable({"contrast",  DataType::Integer, static_cast<int32_t>(value)});
    m_alarm_config.contrast = value;
    WriteStatus();
    LOG(LOG_INFO,"Changed Kinect's contrast to: %d\n",value);

    return 0;
}

int Alarm::ChangeThreshold(int32_t value)
{
    m_detection_config.threshold = static_cast<uint16_t>(value);
    WriteStatus();
    m_detection->UpdateConfig(m_detection_config);
    m_message_broker->SetVariable({"threshold",  DataType::Integer, static_cast<int32_t>(value)});
    LOG(LOG_INFO,"Changed Kinect's threshold to: %d\n",value);

    /* Publish events */
    char message[255];
    sprintf(message, "Threshold changed to %d",value);
    m_message_broker->Publish("event_success",message);

    return 0;
}

int Alarm::ChangeSensitivity(int32_t value)
{
    m_detection_config.sensitivity = static_cast<uint16_t>(value);
    WriteStatus();
    m_detection->UpdateConfig(m_detection_config);
    m_message_broker->SetVariable({"sensitivity",  DataType::Integer, static_cast<int32_t>(value)});
    LOG(LOG_INFO,"Changed Kinect's sensitivity to: %d\n",value);
    
    /* Publish events */
    char message[255];
    sprintf(message, "Sensitivity changed to %d",value);
    m_message_broker->Publish("event_success",message);

    return 0;
}

AlarmDetectionObserver::AlarmDetectionObserver(Alarm& alarm) :
    m_alarm(alarm)
{
    ;
}

AlarmLiveviewObserver::AlarmLiveviewObserver(Alarm& alarm) :
    m_alarm(alarm)
{
    ;
}

void AlarmLiveviewObserver::NewFrame(KinectVideoFrame& frame)
{
    static std::vector<uint8_t> liveview_jpeg;

    /* Convert to jpeg */
    frame.SaveToJpegInMemory(liveview_jpeg, m_alarm.m_alarm_config.brightness, m_alarm.m_alarm_config.contrast);

    /* Convert to base64 */
    const char *base64_jpeg_frame = base64encode(&m_alarm.m_c, liveview_jpeg.data(), liveview_jpeg.size());

    /* Publish in redis channel */
    m_alarm.m_message_broker->Publish("liveview", base64_jpeg_frame);;
}

void AlarmDetectionObserver::IntrusionStarted()
{
    /* Publish events */
    char message[255];
    sprintf(message, "New Intrusion");
    m_alarm.m_message_broker->Publish("event_error",message);
    m_alarm. m_message_broker->Publish("email_send_det","");

    /* Update kinect led */
    m_alarm.m_kinect->ChangeLedColor(LED_RED);
}

void AlarmDetectionObserver::IntrusionStopped(uint32_t frame_num)
{
    char filepath_vid[PATH_MAX];
    char filepath[PATH_MAX];
    sprintf(filepath_vid,"%s/%u_%s",DETECTION_PATH, m_alarm.m_alarm_config.current_detection_number, "capture_vid.mp4");
    sprintf(filepath,"%s/%u_capture.zip", DETECTION_PATH, m_alarm.m_alarm_config.current_detection_number);

    /* Update kinect led */
    m_alarm.UpdateLed();

    /* Publish event */
    char message[255];
    sprintf(message, "newdet %u %u %u", m_alarm.m_alarm_config.current_detection_number, 1000, frame_num);
    m_alarm.m_message_broker->Publish("new_det",message);

    /* Update SQLite db */
    Entry detection_entry = m_alarm.m_detection_table_definition;
    detection_entry[0].value = static_cast<int>(m_alarm.m_alarm_config.current_detection_number);   /*ID*/
    detection_entry[1].value = static_cast<int>(1000); /*DATE*/
    detection_entry[2].value = static_cast<int>(frame_num);   /*DURATION*/
    detection_entry[3].value = std::string(filepath);   /*FILENAME_IMG*/
    detection_entry[4].value = std::string(filepath_vid);   /*FILENAME_VID*/

    if(0 != m_alarm.m_detection_table->InsertItem(detection_entry))
    {
        LOG(LOG_WARNING,"Error creating status table\n");
    }

    /* Update Redis db */
    m_alarm.m_message_broker->SetVariable({"det_numdet",  DataType::Integer, static_cast<int32_t>(m_alarm.m_alarm_config.current_detection_number)});

    /* Package detections */
    char command[PATH_MAX];
    sprintf(command,"cd %s;zip -q %u_capture.zip %u*",DETECTION_PATH, m_alarm.m_alarm_config.current_detection_number, m_alarm.m_alarm_config.current_detection_number);
    system(command);

    /* Change Status */
    m_alarm.m_alarm_config.current_detection_number += 1;
    m_alarm.WriteStatus();
}

void AlarmDetectionObserver::IntrusionFrame(std::shared_ptr<KinectVideoFrame> frame, uint32_t frame_num)
{
    /* Save the frame to JPEG */
    char filepath[PATH_MAX];

    sprintf(filepath,"%s/%u_capture_%d.jpeg",DETECTION_PATH, m_alarm.m_alarm_config.current_detection_number, frame_num);

    if(frame->SaveToJpegInFile(filepath, m_alarm.m_alarm_config.brightness, m_alarm.m_alarm_config.contrast))
    {
        LOG(LOG_ERR,"Error saving video frame\n");
    }
}

