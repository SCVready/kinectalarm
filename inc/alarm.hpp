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
#include "kinect.hpp"
#include "log.hpp"
#include "config.hpp"
#include "jpeg.hpp"
#include "redis_db.hpp"
#include "sqlite_db.hpp"
#include "video_stream.hpp"
#include "video.hpp"
#include "liveview.hpp"
#include "detection.hpp"

/*******************************************************************
 * Defines
 *******************************************************************/
#define DETECTION_THRESHOLD    2000
#define DEPTH_CHANGE_TOLERANCE 10
#define MAX_NUM_DETECTIONS     100
#define NUM_DETECTIONS_FRAMES  5
#define FRAME_INTERVAL_US      200000

#define NUM_DET_PARAMETERS 6 /* TODO: move to another location */
#define NUM_LVW_PARAMETERS 1 /* TODO: move to another location */

/*******************************************************************
 * Structures
 *******************************************************************/
struct sDet_conf
{
    volatile bool is_active;
    uint16_t      threshold;
    uint16_t      tolerance;
    uint16_t      det_num_shots;
    float         frame_interval;
    uint16_t      curr_det_num;
};

enum enumDet_conf
{
    DET_ACTIVE,
    THRESHOLD,
    TOLERANCE,
    DET_NUM_SHOTS,
    FRAME_INTERVAL,
    CURR_DET_NUM,
};

struct sLvw_conf
{
    bool    is_active;
    int16_t tilt;
    int32_t brightness;
    int32_t contrast;
};

enum enumLvw_conf
{
    LVW_ACTIVE,
    TILT,
    BRIGHTNESS,
    CONTRAST,
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
    void IntrusionStopped(uint32_t det_num, uint32_t frame_num) override;
private:
    Alarm& m_alarm;
};

class Alarm
{
    friend AlarmDetectionObserver;
public:
    /**
     * @brief Contructor
     * 
     */
    Alarm();

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
    /* Kinect object */
    std::shared_ptr<Kinect> m_kinect;

    /* Liveview object */
    std::unique_ptr<Liveview> m_liveview;

    /* Detection object */
    std::unique_ptr<Detection> m_detection;

    /* Detection observer object */
    std::shared_ptr<AlarmDetectionObserver> m_detection_observer;

    /* State & config structs */
    struct sDet_conf det_conf;
    struct sLvw_conf lvw_conf;

    void UpdateLed();
    void SetReferenceDepthImage();
    void SetCaptureVideoImage(int num);
    uint32_t CompareDepthFrameToReferenceDepthFrame();

    int InitVarsRedis();

    static void *DetectionThreadHelper(void *context);

    void *DetectionFunction(void);

    template <typename T>
    int ChangeDetStatus(enum enumDet_conf, T value);

    template <typename T>
    int ChangeLvwStatus(enum enumLvw_conf, T value);
};

extern int pipe_fd[2];

#endif /* ALARM_H_ */
