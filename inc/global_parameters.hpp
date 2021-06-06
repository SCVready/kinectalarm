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
#define SOCKET_PATH    "/run/kinectalarm/kinect_alarm_socket"
#define PIPE_PATH      "/run/kinectalarm/liveview_frames_pipe"
#define DETECTION_PATH "/var/detections"
#define CONFIG_PATH    "/etc/kinectalarm/kinectalarm.conf"

#define ALARM_TILT   2000U
#define ALARM_BRIGHTNESS   2000U
#define ALARM_CONTRAST   2000U

#define DETECTION_THRESHOLD   2000U
#define DETECTION_SENSITIVITY 10U
#define DETECTION_COOLDOWN_MS 2000U
#define DETECTION_REFRESH_REFERENCE_INTERVAL_MS 1000U
#define DETECTION_TAKE_DEPTH_FRAME_INTERVAL_MS  10U
#define DETECTION_TAKE_VIDEO_FRAME_INTERVAL_MS  200U

#define LIVEVIEW_FRAME_INTERVAL_MS 100U

#define KINECT_GETFRAMES_TIMEOUT_MS 1000

#define DEPTH_WIDTH    640 /* Fix depth image's width resolution */
#define DEPTH_HEIGHT   480 /* Fix depth image's height resolution */
#define VIDEO_WIDTH    640 /* Fix video image's width resolution */
#define VIDEO_HEIGHT   480 /* Fix video image's height resolution */


#endif /* GLOBAL_PARAMETERS_H_ */
