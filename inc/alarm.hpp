/**
 * @author Alejandro Solozabal
 *
 * @file alarm.hpp
 *
 */

#ifndef ALARM_H_
#define ALARM_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>
#include <memory>

#include "global_parameters.hpp"
#include "message_broker.hpp"
#include "state_persistence.hpp"
#include "kinect.hpp"
#include "log.hpp"
#include "video_stream.hpp"
#include "video.hpp"
#include "liveview.hpp"
#include "detection.hpp"

/*******************************************************************
 * Structures
 *******************************************************************/
struct AlarmConfig
{
    int tilt;
    int brightness;
    int contrast;
    int detection_active;
    int liveview_active;
    int current_detection_number;
};

/*******************************************************************
 * Class declaration
 *******************************************************************/
class Alarm;

class AlarmDetectionObserver : public DetectionObserver
{
public:
    AlarmDetectionObserver(Alarm& alarm);
    void IntrusionStarted() override;
    void IntrusionStopped(uint32_t frame_num) override;
    void IntrusionFrame(std::shared_ptr<KinectVideoFrame> frame, uint32_t frame_num) override;
private:
    Alarm& m_alarm;
};

class AlarmLiveviewObserver : public LiveviewObserver
{
public:
    AlarmLiveviewObserver(Alarm& alarm);
    void NewFrame(KinectVideoFrame& frame) override;
private:
    Alarm& m_alarm;
};

class Alarm
{
    friend AlarmDetectionObserver;
    friend AlarmLiveviewObserver;
public:
    /**
     * @brief Contructor
     * 
     */
    Alarm(std::shared_ptr<IMessageBroker> message_broker, std::shared_ptr<IDatabase> data_base);

    /**
     * @brief Destructor
     * 
     */
    virtual ~Alarm();

    /**
     * @brief Initialization
     * 
     */
    int Init();

    /**
     * @brief Termination
     * 
     */
    int Term();

    /**
     * @brief Start detection
     * 
     */
    int StartDetection();

    /**
     * @brief Stop detection
     * 
     */
    int StopDetection();

    /**
     * @brief Check if kinect is running
     * 
     */
    bool IsDetectionRunning();

    /**
     * @brief Start liveview
     * 
     */
    int StartLiveview();

    /**
     * @brief Stop live view
     * 
     */
    int StopLiveview();

    /**
     * @brief Check if kinect is running
     * 
     */
    bool IsLiveviewRunning();

    /**
     * @brief Get number of detections
     * 
     */
    int GetNumDetections();

    /**
     * @brief Reset number of detection
     * 
     */
    int ResetDetection();

    /**
     * @brief Reset number of detection
     * 
     */
    int DeleteDetection(int id);

    /**
     * @brief Change Kinect's tilt
     * 
     */
    int ChangeTilt(double tilt);

    /**
     * @brief Change Kinect's contrast
     * 
     */
    int ChangeContrast(int32_t value);

    /**
     * @brief Change Kinect's brightness
     * 
     */
    int ChangeBrightness(int32_t value);

    /**
     * @brief Change detection's threshold
     * 
     */
    int ChangeThreshold(int32_t value);

    /**
     * @brief Change detection's sensitivity
     * 
     */
    int ChangeSensitivity(int32_t value);

private:

    struct sBase64encode_context m_base64_encoder_context;

    /* Kinect object */
    std::shared_ptr<IKinect> m_kinect;

    /* Liveview object */
    std::shared_ptr<IAlarmModule> m_liveview;

    /* Liveview observer object */
    std::shared_ptr<AlarmLiveviewObserver> m_liveview_observer;

    /* Detection object */
    std::shared_ptr<IAlarmModule> m_detection;

    /* Detection observer object */
    std::shared_ptr<AlarmDetectionObserver> m_detection_observer;

    /* MessageBroker object */
    std::shared_ptr<IMessageBroker> m_message_broker;

    /* MessageBroker observer object */
    std::shared_ptr<IMessageBroker> m_message_broker_observer;

    /* Database object */
    std::shared_ptr<IDatabase> m_data_base;

    AlarmConfig m_alarm_config{
        .tilt = ALARM_TILT,
        .brightness = ALARM_BRIGHTNESS,
        .contrast = ALARM_CONTRAST,
        .detection_active = 0,
        .liveview_active = 0,
        .current_detection_number = 0
    };

    DetectionConfig m_detection_config = {
        .threshold = DETECTION_THRESHOLD,
        .sensitivity = DETECTION_SENSITIVITY,
        .cooldown_ms = DETECTION_COOLDOWN_MS,
        .refresh_reference_interval_ms = DETECTION_REFRESH_REFERENCE_INTERVAL_MS,
        .take_depth_frame_interval_ms = DETECTION_TAKE_DEPTH_FRAME_INTERVAL_MS,
        .take_video_frame_interval_ms = DETECTION_TAKE_VIDEO_FRAME_INTERVAL_MS
    };

    LiveviewConfig m_liveview_config = {
        .video_frame_interval_ms = LIVEVIEW_FRAME_INTERVAL_MS
    };

    std::shared_ptr<IDataTable> m_detection_table;
    std::shared_ptr<IDataTable> m_status_table;

    uint16_t threshold;
    uint16_t sensitivity;
    uint32_t cooldown_ms;
    uint32_t refresh_reference_interval_ms;
    uint32_t take_depth_frame_interval_ms;
    uint32_t take_video_frame_interval_ms;

    const Entry m_status_table_definition = {
        {"ID",              DataType::Integer,},
        {"TILT",            DataType::Integer,},
        {"BRIGHTNESS",      DataType::Integer,},
        {"CONTRAST",        DataType::Integer,},
        {"DET_ACTIVE",      DataType::Integer,},
        {"LVW_ACTIVE",      DataType::Integer,},
        {"CURRENT_DET_NUM", DataType::Integer,},
        {"DET_THRESHOLD",   DataType::Integer,},
        {"DET_SENSITIVITY", DataType::Integer,},
        {"DET_COOLDOWN_MS", DataType::Integer,},
        {"DET_REFRESH_REFERENCE_INTERVAL_MS", DataType::Integer,},
        {"DET_TAKE_DEPTH_FRAME_INTERVAL_MS",  DataType::Integer,},
        {"DET_TAKE_VIDEO_FRAME_INTERVAL_MS",  DataType::Integer,},
        {"LVW_VIDEO_FRAME_INTERVAL_MS",       DataType::Integer,}
    };

    const Entry m_detection_table_definition = {
        {"ID",           DataType::Integer,},
        {"DATE",         DataType::Integer,},
        {"DURATION",     DataType::Integer,},
        {"FILENAME_IMG", DataType::String,},
        {"FILENAME_VID", DataType::String,}
    };

    int UpdateLed();

    int ReadStatus();
    int WriteStatus();
    int CreateStatus();

    int InitVarsRedis();
    int InitStatePersistenceVars();
};

#endif /* ALARM_H_ */
