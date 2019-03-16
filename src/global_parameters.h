/*
 * global_parameters.h
 *
 *  Created on: Mar 13, 2019
 *      Author: scvready
 */

#ifndef GLOBAL_PARAMETERS_H_
#define GLOBAL_PARAMETERS_H_

// Paths

#define SOCKET_PATH 		"/etc/kinectalarm/kinect_alarm_socket"
#define DETECTION_PATH		"/var/detections"
#define CONFIG_PATH			"/etc/kinectalarm/kinectalarm.conf"
#define PIPE_PATH			"/etc/kinectalarm/liveview_frames_pipe"

// Resolution
#define DEPTH_WIDTH		640 // Depth image's width resolution
#define DEPTH_HEIGHT 	480 // Depth image's height resolution
#define VIDEO_WIDTH		640 // Video image's width resolution
#define VIDEO_HEIGHT	480 // Video image's height resolution


#endif /* GLOBAL_PARAMETERS_H_ */
