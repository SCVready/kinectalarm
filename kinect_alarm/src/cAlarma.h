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
#include <syslog.h>

#include <iostream>

#include <FreeImage.h>

#include "cKinect.h"
#include "log.h"
#include "config.h"

//// Defines ////

#define PATH					"detections"
#define CONF_PATH				"config.xml"

#define DETECTION_THRESHOLD 	2000
#define DEPTH_CHANGE_TOLERANCE 	10
#define MAX_NUM_DETECTIONS 		100
#define NUM_DETECTIONS_FRAMES 	5
#define FRAME_INTERVAL_US		200000

#define NUM_DET_PARAMETERS 6 //TODO CHANGE LOCATION
#define NUM_LVW_PARAMETERS 1 //TODO CHANGE LOCATION

//// Structs ////

struct sDet_conf
{
	volatile bool		is_active;
	uint16_t			threshold;
	uint16_t			tolerance;
	uint16_t			det_num_shots;
	float				frame_interval;
	uint16_t			curr_det_num;
};

enum enumDet_conf
{
	IS_ACTIVE,
	THRESHOLD,
	TOLERANCE,
	DET_NUM_SHOTS,
	FRAME_INTERVAL,
	CURR_DET_NUM,
};

struct sLvw_conf //TODO
{
	bool		is_active;
};

//// Class ////

class cAlarma {
public:
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

	/** @brief Get number of detections */
	int get_num_detections();

	/** @brief Reset number of detection */
	int reset_detection();

private:

	//// Variables ////

	uint16_t* video_frames[NUM_DETECTIONS_FRAMES];
	uint16_t* reff_depth_frame;
	uint16_t* depth_frame;
	uint16_t* diff_depth_frame;

	pthread_t detection_thread;
	class cKinect kinect;
	//volatile bool detection_running; // Flag to control detection logic
	//volatile bool liveview_running;// Flag to control liveview logic

	struct sDet_conf det_conf;
	struct sLvw_conf lvw_conf;

	//// Functions ////

	void update_led();
	bool save_depth_frame_to_bmp(uint16_t* depth_frame,char *filename);
	bool save_video_frame_to_bmp(uint16_t* video_frame,char *filename);
	bool save_video_frames_to_gif(uint16_t** video_frames_array, int num_frames, float frame_interval, char *filename);
	void set_reference_depth_image();
	void set_capture_video_image(int num);
	uint32_t compare_depth_frame_to_reference_depth_image();
	bool init_num_detection();
	bool delete_detections();
	static void *detection_thread_helper(void *context);
	void *detection(void);

	template <typename T>
	int change_det_status(enum enumDet_conf, T value);
};

#endif /* CALARMA_H_ */
