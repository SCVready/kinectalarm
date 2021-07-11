/**
 * @author Alejandro Solozabal
 *
 * @file global_parameters.hpp
 *
 */

#ifndef GLOBAL_PARAMETERS_H_
#define GLOBAL_PARAMETERS_H_

/*******************************************************************
 * Defines
 *******************************************************************/
#define KINECTALARM_VERSION "0.11"

#define DETECTION_PATH "/var/detections"
#define REDIS_DB_PATH  "/tmp/redis.sock"
#define SQLITE_DB_PATH "/etc/kinectalarm/detections.db"

#define WATCHDOG_TIMEOUT_S  2U
#define WATCHDOG_REFRESH_MS 1000U

#define REDIS_COMMAND_CHANNEL        "kinectalarm"
#define REDIS_EVENT_INFO_CHANNEL     "event_info"
#define REDIS_EVENT_SUCCESS_CHANNEL  "event_success"
#define REDIS_EVENT_ERROR_CHANNEL    "event_error"
#define REDIS_LIVEFRAMES_CHANNEL     "liveview"
#define REDIS_DET_INTRUSION_CHANNEL  "new_det"
#define REDIS_DET_EMAIL_SEND_CHANNEL "email_send_det"

#define ALARM_TILT       0
#define ALARM_BRIGHTNESS 1000
#define ALARM_CONTRAST   0

#define DETECTION_THRESHOLD   2000U
#define DETECTION_SENSITIVITY 10U
#define DETECTION_COOLDOWN_MS 2000U
#define DETECTION_REFRESH_REFERENCE_INTERVAL_MS 1000U
#define DETECTION_TAKE_DEPTH_FRAME_INTERVAL_MS  10U
#define DETECTION_TAKE_VIDEO_FRAME_INTERVAL_MS  200U

#define LIVEVIEW_FRAME_INTERVAL_MS 150U

#define KINECT_GETFRAMES_TIMEOUT_MS 1000U

#define DEPTH_WIDTH    640U
#define DEPTH_HEIGHT   480U
#define VIDEO_WIDTH    640U
#define VIDEO_HEIGHT   480U


#endif /* GLOBAL_PARAMETERS_H_ */
