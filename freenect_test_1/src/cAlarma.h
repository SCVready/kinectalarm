/*
 * cAlarma.h
 *
 *  Created on: 18 ago. 2018
 *      Author: asolo
 */

#include "cKinect.h"

#ifndef CALARMA_H_
#define CALARMA_H_

#define DETECTION_THRESHOLD 2000
#define DEPTH_CHANGE_TOLERANCE 10

#define MAX_NUM_DETECTIONS 100
#define NUM_DETECTIONS_FRAMES 5

class cAlarma {
public:
	cAlarma();
	virtual ~cAlarma();

	int init();
	int deinit();

	int run();
	int stop();

	bool save_depth_frame_to_bmp(uint16_t* depth_frame,char *filename);
	bool save_video_frame_to_bmp(uint16_t* video_frame,char *filename);

	void set_reference_depth_image();
	void set_capture_video_image(int num);
	uint32_t compare_depth_frame_to_reference_depth_image();
	bool init_num_detection();

	static void *detection_thread_helper(void *context);
	void *detection(void);

	uint16_t* video_frames[NUM_DETECTIONS_FRAMES];
	uint16_t num_detections;
	uint16_t* reff_depth_frame;
	uint16_t* depth_frame;
	uint16_t* diff_depth_frame;

private:

	class cKinect kinect;
	volatile bool running;

	pthread_t detection_thread;

};

#endif /* CALARMA_H_ */
