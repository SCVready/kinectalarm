/**
  * @file cAlarma.h
  * @author Alejandro Solozabal
  */

#ifndef CALARMA_H_
#define CALARMA_H_

//// Includes ////
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include "FreeImage.h"
#include "cKinect.h"

//// Defines ////

#define DETECTION_THRESHOLD 2000
#define DEPTH_CHANGE_TOLERANCE 10
#define MAX_NUM_DETECTIONS 100
#define NUM_DETECTIONS_FRAMES 5

//// Class ////

class cAlarma {
public:

	//// Variables ////

	uint16_t* video_frames[NUM_DETECTIONS_FRAMES];
	uint16_t num_detections;
	uint16_t* reff_depth_frame;
	uint16_t* depth_frame;
	uint16_t* diff_depth_frame;

	//// Functions ////

	/** @brief Constructor */
	cAlarma();

	/** @brief Destructor */
	virtual ~cAlarma();

	/** @brief Initializer */
	int init();

	/** @brief Deinitializer */
	int deinit();

	/** @brief Start detection */
	int start_detection();

	/** @brief Stop detection */
	int stop_detection();

	/** @brief Check if kinect is running */
	bool is_detection_running();

	/** @brief Start live view */
	int start_liveview();

	/** @brief Stop live view */
	int stop_liveview();

	/** @brief Check if kinect is running */
	bool is_liveview_running();

private:

	//// Variables ////

	pthread_t detection_thread;
	class cKinect kinect;
	volatile bool detection_running; // Flag to control detection logic
	volatile bool liveview_running;// Flag to control liveview logic

	//// Functions ////

	void update_led();
	bool save_depth_frame_to_bmp(uint16_t* depth_frame,char *filename);
	bool save_video_frame_to_bmp(uint16_t* video_frame,char *filename);
	void set_reference_depth_image();
	void set_capture_video_image(int num);
	uint32_t compare_depth_frame_to_reference_depth_image();
	bool init_num_detection();
	static void *detection_thread_helper(void *context);
	void *detection(void);
};

#endif /* CALARMA_H_ */
