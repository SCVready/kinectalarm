/*
 * cKinect.h
 *
 *  Created on: 10 ago. 2018
 *      Author: asolo
 */

#ifndef CKINECT_H_
#define CKINECT_H_

// Includes
#include <semaphore.h>
#include <thread>
#include "libfreenect.h"
#include "libfreenect_sync.h"

// Defines
#define LOCAL_PATH		"/home/pi/detections"

#define MAX_TILT_WAIT 	10

#define DEPTH_WIDTH		640
#define DEPTH_HEIGHT 	480
#define VIDEO_WIDTH		640 //1280
#define VIDEO_HEIGHT	480 //1024

class cKinect {

private:
	// Variables
	freenect_context* kinect_ctx;
	freenect_device* kinect_dev;
	pthread_t process_event_thread;

	bool is_kinect_initialize;

	uint16_t buffer_depth[DEPTH_WIDTH*DEPTH_HEIGHT];
	uint16_t buffer_video[VIDEO_WIDTH*VIDEO_HEIGHT];

	static volatile bool done_depth;
	static volatile bool done_video;

	// Functions
	static void video_cb(freenect_device* dev, void* data, uint32_t timestamp);
	static void depth_cb(freenect_device* dev, void* data, uint32_t timestamp);

	volatile bool running;


public:
	// Variables
	static uint16_t* temp_depth_frame_raw;
	static uint16_t* temp_video_frame_raw;


	static pthread_mutex_t depth_lock;
	static pthread_mutex_t video_lock;

	static pthread_cond_t depth_ready;
	static pthread_cond_t video_ready;

	// Functions
	cKinect();
	virtual ~cKinect();

	bool init();
	bool deinit();

	int run();
	int stop();

	int get_depth_frame(uint16_t *depth_frame);
	int get_video_frame(uint16_t *video_frame);

	bool change_tilt(double tilt_angle); // range [-61,61]
	void change_led_color(freenect_led_options color);

	void *kinect_process_events(void);
	static void *kinect_process_events_helper(void *context);

};

#endif /* CKINECT_H_ */

bool check_dir_exist(char *dir);
int create_dir(char *dir);
