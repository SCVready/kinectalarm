/**
 * @author Alejandro Solozabal
 *
 * @file alarm.cpp
 *
 */

/*******************************************************************
 * Includes
 *******************************************************************/
#include <filesystem>

#include "alarm.hpp"
#include "kinect_factory.hpp"
#include "alarm_module_factory.hpp"
#include "message_broker_factory.hpp"
#include "state_persistence_factory.hpp"

/*******************************************************************
 * Class definition
 *******************************************************************/
Alarm::Alarm(std::shared_ptr<IMessageBroker> message_broker, std::shared_ptr<IDatabase> data_base) :
    m_message_broker(message_broker),
    m_data_base(data_base)
{
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
    int ret_val = -1;

    if(0 != InitStatePersistenceVars())
    {
        LOG(LOG_ERR, "Error: couldn't initialize StatePersistence vars\n");
    }
    else if(0 != InitVarsRedis())
    {
        LOG(LOG_ERR, "Error: couldn't initialize variables in Redis db\n");
    }
    else if(0 != m_kinect->Init())
    {
        LOG(LOG_ERR, "Error: couldn't initialize Kinect\n");
    }
    else
    {
        /* Update kinect led */
        UpdateLed();

        /* Apply status */
        if(m_alarm_config.detection_active && (0 != StartDetection()))
        {
            LOG(LOG_ERR, "Error: couldn't start Detection module\n");
        }
        else if(m_alarm_config.liveview_active && (0 != StartLiveview()))
        {
            LOG(LOG_ERR, "Error: couldn't start Liveview module\n");
        }
        else if(m_kinect->ChangeTilt(m_alarm_config.tilt))
        {
            LOG(LOG_ERR, "Error: couldn't change Kinect's tilt\n");
        }
        else
        {
            LOG(LOG_NOTICE, "Alarm initialized successfully\n");
            ret_val = 0;
        }
    }

    return ret_val;
}

int Alarm::Term()
{
    int ret_val = 0;

    if(m_detection->IsRunning() && (0 != m_detection->Stop()))
    {
        LOG(LOG_ERR, "Error stoping Detection module\n");
        ret_val = -1;
    }

    if(m_liveview->IsRunning() && (0 != m_liveview->Stop()))
    {
        LOG(LOG_ERR, "Error stoping Liveview module\n");
        ret_val = -1;
    }

    if(m_kinect->IsRunning() && (0 != m_kinect->Stop()))
    {
        LOG(LOG_ERR, "Error stoping Kinect\n");
        ret_val = -1;
    }

    if(0 != m_kinect->Term())
    {
        LOG(LOG_ERR, "Error terminating Kinect\n");
        ret_val = -1;
    }

    return ret_val;
}

int Alarm::StartDetection()
{
    int ret_val = -1;

    if(!m_kinect->IsRunning() && (0 != m_kinect->Start()))
    {
        LOG(LOG_ERR, "Error starting Kinect\n");
    }
    else if(m_detection->IsRunning())
    {
        LOG(LOG_INFO, "Detection module already started\n");
    }
    else
    {
        if(0 != m_detection->Start())
        {
            LOG(LOG_ERR, "Detection module start returned an error\n");
        }
        else
        {
            /* Update Persistence DB */
            m_alarm_config.detection_active = 1;
            if(0 != WriteStatus())
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
            }

            /* Update Cache DB */
            if(0 != m_message_broker->SetVariable({"det_status",  DataType::Integer, 1}))
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
            }

            /* Update led */
            if(0 != UpdateLed())
            {
                LOG(LOG_WARNING, "Couldn't update Kinect's led\n");
            }

            /* Publish event */
            if(0 != m_message_broker->Publish(REDIS_EVENT_INFO_CHANNEL, "Detection started"))
            {
                LOG(LOG_WARNING, "Couldn't publish event\n");
            }

            LOG(LOG_NOTICE, "Detection module started\n");

            ret_val = 0;
        }
    }

    return ret_val;
}

int Alarm::StopDetection()
{
    int ret_val = -1;

    if(m_detection->IsRunning())
    {
        if(0 != m_detection->Stop())
        {
            LOG(LOG_ERR, "Error stopping Detection module\n");
        }
        else
        {
            /* Update Persistence DB */
            m_alarm_config.detection_active = 0;
            if(0 != WriteStatus())
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
            }

            /* Update Cache DB */
            if(0 != m_message_broker->SetVariable({"det_status",  DataType::Integer, 0}))
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
            }

            /* Update led */
            if(0 != UpdateLed())
            {
                LOG(LOG_WARNING, "Couldn't update Kinect's led\n");
            }

            /* Publish event */
            if(0 != m_message_broker->Publish(REDIS_EVENT_INFO_CHANNEL, "Detection stopped"))
            {
                LOG(LOG_WARNING, "Couldn't publish event\n");
            }

            LOG(LOG_NOTICE, "Detection module stopped\n");

            /* Stop Kinect if possible */
            if(!m_detection->IsRunning() && !m_liveview->IsRunning())
            {
                if(0 != m_kinect->Stop())
                {
                    LOG(LOG_WARNING, "Error stopping Kinect\n");
                }
            }

            ret_val = 0;
        }
    }
    else
    {
        LOG(LOG_INFO, "Detection module already stopped\n");
    }

    return ret_val;
}

int Alarm::StartLiveview()
{
    int ret_val = -1;

    if(!m_kinect->IsRunning() && (0 != m_kinect->Start()))
    {
        LOG(LOG_ERR, "Error starting Kinect\n");
    }
    if(!m_liveview->IsRunning())
    {
        if(0 != m_liveview->Start())
        {
            LOG(LOG_ERR, "Liveview module start returned an error\n");
        }
        else
        {
            /* Update Persistence DB */
            m_alarm_config.liveview_active = 1;
            if(0 != WriteStatus())
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
            }

            /* Update Cache DB */
            if(0 != m_message_broker->SetVariable({"lvw_status",  DataType::Integer, 1}))
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
            }

            /* Update led */
            if(0 != UpdateLed())
            {
                LOG(LOG_WARNING, "Couldn't update Kinect's led\n");
            }

            /* Publish event */
            if(0 != m_message_broker->Publish(REDIS_EVENT_INFO_CHANNEL,"Liveview started"))
            {
                LOG(LOG_WARNING, "Couldn't publish event\n");
            }

            LOG(LOG_NOTICE, "Liveview module started\n");

            ret_val = 0;
        }
    }

    return ret_val;
}

int Alarm::StopLiveview()
{
    int ret_val = -1;

    if(m_liveview->IsRunning())
    {
        if(0 != m_liveview->Stop())
        {
            LOG(LOG_ERR, "Error stopping Liveview module\n");
        }
        else
        {
            /* Update Persistence DB */
            m_alarm_config.liveview_active = 0;
            if(0 != WriteStatus())
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
            }

            /* Update Cache DB */
            if(0 != m_message_broker->SetVariable({"lvw_status",  DataType::Integer, 0}))
            {
                LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
            }

            /* Update led */
            if(0 != UpdateLed())
            {
                LOG(LOG_WARNING, "Couldn't update Kinect's led\n");
            }

            /* Publish event */
            if(0 != m_message_broker->Publish(REDIS_EVENT_INFO_CHANNEL, "Liveview stopped"))
            {
                LOG(LOG_WARNING, "Couldn't publish event\n");
            }

            LOG(LOG_NOTICE, "Liveview module stopped\n");

            /* Stop Kinect if possible */
            if(!m_detection->IsRunning() && !m_liveview->IsRunning())
            {
                if(0 != m_kinect->Stop())
                {
                    LOG(LOG_WARNING, "Error stopping Kinect\n");
                }
            }

            ret_val = 0;
        }
    }
    else
    {
        LOG(LOG_INFO, "Detection module already stopped\n");
    }

    return ret_val;
}

int Alarm::UpdateLed()
{
    int ret_val = -1;
    freenect_led_options color = LED_OFF;

    if(m_detection->IsRunning())
    {
        color = LED_YELLOW;
    }
    else if(m_liveview->IsRunning())
    {
        color = LED_GREEN;
    }
    else
    {
        color = LED_OFF;
    }

    if(0 != m_kinect->ChangeLedColor(color))
    {
        LOG(LOG_WARNING,"Kinect ChangeLedColor returned an error\n");
    }
    else
    {
        LOG(LOG_INFO,"Kinect led changed\n");
        ret_val = 0;
    }

    return ret_val;
}

int Alarm::GetNumDetections()
{
    return m_alarm_config.current_detection_number;
}

int Alarm::ResetDetection()
{
    /* Update Persistence DB */
    m_detection_table->DeleteAllItems();
#if 0 /* TODO */
    /* Delete all files from Detection path */
    for(const auto& entry : std::filesystem::directory_iterator(DETECTION_PATH))
    {
        std::filesystem::remove_all(entry.path());
    }
#else
    DeleteAllFilesFromDirectory(DETECTION_PATH);
#endif
    /* Publish event */
    if(0 != m_message_broker->Publish(REDIS_EVENT_SUCCESS_CHANNEL, "Deleted all instrusions"))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

    LOG(LOG_INFO,"Deleted all detection entries\n");

    return 0;
}

int Alarm::DeleteDetection(int id)
{
#if 0 /* TODO */
    /* Delete all files from Detection id, "{id}*" */
    for(auto& entry: std::filesystem::directory_iterator(DETECTION_PATH))
    {
        std::string filename = entry.path().filename().string();

        if(0 == filename.find(std::to_string(id)))
        {
            std::filesystem::remove(entry);
        }
    }
#else
    std::string command = "rm -rf " + std::string(DETECTION_PATH) + "/" + std::to_string(id) + "_*";
    system(command.c_str());
#endif

    /* Update Persistence DB */
    Entry delete_entry = m_detection_table_definition;
    delete_entry[0].value = id; /* ID */
    m_detection_table->DeleteItem(delete_entry);

    /* Publish event */
    if(0 != m_message_broker->Publish(REDIS_EVENT_SUCCESS_CHANNEL, "Deleted intrusion"))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

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
        LOG(LOG_ERR,"Error creating Detection table \n");
    }
    else if (nullptr == (m_status_table = StatePersistenceFactory::CreateDatatable(m_data_base, "STATUS", m_status_table_definition)))
    {
        LOG(LOG_ERR,"Error creating Status table \n");
    }
    else
    {
        if(0 != ReadStatus())
        {
            LOG(LOG_WARNING,"Status Table not present\n");

            if(0 != CreateStatus())
            {
                LOG(LOG_ERR,"Error creating a new Status table\n");
            }
            else
            {
                LOG(LOG_INFO,"New Status table created with default values\n");
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

        LOG(LOG_INFO,"Status table read\n");
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
        LOG(LOG_WARNING,"InsertItem returned error\n");
    }
    else
    {
        ret_val = 0;
    }

    return ret_val;
}

int Alarm::ChangeTilt(double value)
{
    int ret_val = 0;

    if(0 != m_kinect->ChangeTilt(value))
    {
        LOG(LOG_ERR, "Kinect returned Error on trying to change the tilt\n");
        ret_val = -1;
    }
    else
    {
        m_alarm_config.tilt = value;

        /* Update Cache DB */
        if(0 != m_message_broker->SetVariable({"tilt",  DataType::Integer, static_cast<int32_t>(value)}))
        {
            LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
        }

        /* Update Persistence DB */
        if(0 != WriteStatus())
        {
            LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
        }

        LOG(LOG_INFO,"Changed Kinect's tilt to: %.2f\n", value);
    }

    return ret_val;
}

int Alarm::ChangeBrightness(int32_t value)
{
    m_alarm_config.brightness = value;

    /* Update Cache DB */
    if(0 != m_message_broker->SetVariable({"brightness",  DataType::Integer, static_cast<int32_t>(value)}))
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
    }

    /* Update Persistence DB */
    if(0 != WriteStatus())
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
    }

    LOG(LOG_INFO,"Changed Kinect's brightness to: %d\n",value);

    return 0;
}

int Alarm::ChangeContrast(int32_t value)
{
    m_alarm_config.contrast = value;

    /* Update Cache DB */
    if(0 != m_message_broker->SetVariable({"contrast",  DataType::Integer, static_cast<int32_t>(value)}))
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
    }

    /* Update Persistence DB */
    if(0 != WriteStatus())
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
    }

    LOG(LOG_INFO,"Changed Kinect's contrast to: %d\n",value);

    return 0;

}

int Alarm::ChangeThreshold(int32_t value)
{
    m_detection_config.threshold = static_cast<uint16_t>(value);
    m_detection->UpdateConfig(m_detection_config);

    /* Update Cache DB */
    if(0 != m_message_broker->SetVariable({"threshold",  DataType::Integer, static_cast<int32_t>(value)}))
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
    }

    /* Update Persistence DB */
    if(0 != WriteStatus())
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
    }

    /* Publish event */
    if(0 != m_message_broker->Publish(REDIS_EVENT_SUCCESS_CHANNEL, std::string("Threshold changed to ") + std::to_string(value)))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

    LOG(LOG_INFO,"Changed Kinect's threshold to: %d\n",value);

    return 0;
}

int Alarm::ChangeSensitivity(int32_t value)
{
    m_detection_config.sensitivity = static_cast<uint16_t>(value);
    m_detection->UpdateConfig(m_detection_config);

    /* Update Cache DB */
    if(0 != m_message_broker->SetVariable({"sensitivity",  DataType::Integer, static_cast<int32_t>(value)}))
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
    }

    /* Update Persistence DB */
    if(0 != WriteStatus())
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Persisten DB\n");
    }

    /* Publish event */
    if(0 != m_message_broker->Publish(REDIS_EVENT_SUCCESS_CHANNEL, std::string("Sensitivity changed to ") + std::to_string(value)))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

    LOG(LOG_INFO,"Changed Kinect's sensitivity to: %d\n",value);

    return 0;
}

AlarmDetectionObserver::AlarmDetectionObserver(Alarm& alarm) :
    m_alarm(alarm)
{
}

AlarmLiveviewObserver::AlarmLiveviewObserver(Alarm& alarm) :
    m_alarm(alarm)
{
}

void AlarmLiveviewObserver::NewFrame(KinectVideoFrame& frame)
{
    static std::vector<uint8_t> liveview_jpeg;

    /* Convert to jpeg */
    if(0 != frame.SaveToJpegInMemory(liveview_jpeg, m_alarm.m_alarm_config.brightness, m_alarm.m_alarm_config.contrast))
    {
        LOG(LOG_ERR, "Couldn't convert frame to Jpeg\n");
    }
    else
    {
        /* Convert to base64 */
        std::string base64_jpeg_frame = m_alarm.m_base64_encoder.Encode(std::string(liveview_jpeg.begin(), liveview_jpeg.end()));

        /* Publish event */
        if(0 != m_alarm.m_message_broker->Publish(REDIS_LIVEFRAMES_CHANNEL, base64_jpeg_frame))
        {
            LOG(LOG_WARNING, "Couldn't publish event\n");
        }
    }
}

void AlarmDetectionObserver::IntrusionStarted()
{
    /* Publish events */
    if(0 != m_alarm.m_message_broker->Publish(REDIS_DET_EMAIL_SEND_CHANNEL, ""))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

    if(0 != m_alarm.m_message_broker->Publish(REDIS_EVENT_ERROR_CHANNEL, "New Intrusion"))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

    /* Update kinect led */
    if(0 != m_alarm.m_kinect->ChangeLedColor(LED_RED))
    {
        LOG(LOG_ERR, "Error couldn't change Kinect's Led color\n");
    }
}

void AlarmDetectionObserver::IntrusionStopped(uint32_t frame_num)
{
    /* Update kinect led */
    m_alarm.UpdateLed();

    /* Publish event */
    std::string message = std::string("newdet ") + std::to_string(m_alarm.m_alarm_config.current_detection_number) + " " +
                          std::to_string(1000) + " " + std::to_string(frame_num);

    if(0 != m_alarm.m_message_broker->Publish(REDIS_DET_INTRUSION_CHANNEL, message))
    {
        LOG(LOG_WARNING, "Couldn't publish event\n");
    }

    /* Update SQLite db */
    Entry detection_entry = m_alarm.m_detection_table_definition;
    detection_entry[0].value = static_cast<int>(m_alarm.m_alarm_config.current_detection_number); /*ID*/
    detection_entry[1].value = static_cast<int>(1000); /*DATE*/
    detection_entry[2].value = static_cast<int>(frame_num); /*DURATION*/
    detection_entry[3].value = std::string(DETECTION_PATH) + "/" + std::to_string(m_alarm.m_alarm_config.current_detection_number) + "_capture.zip"; /*FILENAME_IMG*/
    detection_entry[4].value = std::string(DETECTION_PATH) + "/" + std::to_string(m_alarm.m_alarm_config.current_detection_number) + "_capture_vid.mp4"; /*FILENAME_VID*/

    if(0 != m_alarm.m_detection_table->InsertItem(detection_entry))
    {
        LOG(LOG_WARNING,"Error creating status table\n");
    }

    /* Update Redis db */
    if(0 != m_alarm.m_message_broker->SetVariable({"det_numdet",  DataType::Integer, static_cast<int32_t>(m_alarm.m_alarm_config.current_detection_number)}))
    {
        LOG(LOG_WARNING, "Couldn't write Status in the Cache DB\n");
    }

    /* Package detections */
    /* TODO dont use commenad, use a lib? */
    char command[PATH_MAX];
    sprintf(command,"cd %s;zip -q %u_capture.zip %u*",DETECTION_PATH, m_alarm.m_alarm_config.current_detection_number, m_alarm.m_alarm_config.current_detection_number);
    system(command);

    /* Change Status */
    m_alarm.m_alarm_config.current_detection_number += 1;
    m_alarm.WriteStatus();
}

void AlarmDetectionObserver::IntrusionFrame(KinectVideoFrame& frame, uint32_t frame_num)
{
    /* Save the frame to JPEG */
    std::string filepath = std::string(DETECTION_PATH) + "/" + std::to_string(m_alarm.m_alarm_config.current_detection_number) + 
                           "_capture_" + std::to_string(frame_num) + ".jpeg";

    if(frame.SaveToJpegInFile(filepath, m_alarm.m_alarm_config.brightness, m_alarm.m_alarm_config.contrast))
    {
        LOG(LOG_ERR,"Error saving intrusion Jpeg frame to file\n");
    }
}

