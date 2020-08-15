/**
 * @author Alejandro Solozabal
 *
 * @file global_parameters.hpp
 *
 */

#ifndef GLOBAL_PARAMETERS_H_
#define GLOBAL_PARAMETERS_H_

// Paths

#define SOCKET_PATH    "/run/kinectalarm/kinect_alarm_socket"
#define PIPE_PATH      "/run/kinectalarm/liveview_frames_pipe"
#define DETECTION_PATH "/var/detections"
#define CONFIG_PATH    "/etc/kinectalarm/kinectalarm.conf"


// Resolution
#define DEPTH_WIDTH    640 // Depth image's width resolution
#define DEPTH_HEIGHT   480 // Depth image's height resolution
#define VIDEO_WIDTH    640 // Video image's width resolution
#define VIDEO_HEIGHT   480 // Video image's height resolution


#endif /* GLOBAL_PARAMETERS_H_ */
