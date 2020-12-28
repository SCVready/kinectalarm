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

#define DETECTION_THRESHOLD    2000
#define DEPTH_CHANGE_TOLERANCE 10
#define MAX_NUM_DETECTIONS     100
#define NUM_DETECTIONS_FRAMES  5
#define FRAME_INTERVAL_US      200000

#define DEPTH_WIDTH    640 /* Fix depth image's width resolution */
#define DEPTH_HEIGHT   480 /* Fix depth image's height resolution */
#define VIDEO_WIDTH    640 /* Fix video image's width resolution */
#define VIDEO_HEIGHT   480 /* Fix video image's height resolution */


#endif /* GLOBAL_PARAMETERS_H_ */
