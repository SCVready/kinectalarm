/**
  * @file cAlarma.h
  * @author Alejandro Solozabal
  */

#ifndef CALARM_H_
#define CALARM_H_

//// Includes ////

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

#include "global_parameters.h"
#include "cKinect.h"
#include "log.h"
#include "config.h"
#include "jpeg.h"
#include "video.h"

//// Defines ////

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
	bool				is_active;
};

//// Class ////

class cAlarm {
public:
	//// Functions ////

	/** @brief Constructor */
	cAlarm();

	/** @brief Destructor */
	virtual ~cAlarm();

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

	class cKinect kinect;

	// Frame pointer for detection
	uint16_t* reff_depth_frame;
	uint16_t* depth_frame;
	uint16_t* diff_depth_frame;
	uint16_t* video_frames[NUM_DETECTIONS_FRAMES];

	// Frame's timestamp for detection
	uint32_t reff_depth_timestamp;
	uint32_t depth_timestamp;
	uint32_t video_timestamp;

	// Frame pointer for detection
	uint16_t* liveview_frame;
	uint8_t* liveview_jpeg;

	// Frame pointer for detection
	uint32_t liveview_timestamp;

	// Frame jpeg buffer out
	uint8_t* liveview_buffer_out;

	// Threads
	pthread_t detection_thread;
	pthread_t liveview_thread;

	volatile bool detection_running; // Flag to control detection logic
	volatile bool liveview_running;// Flag to control liveview logic

	// State/config structs
	struct sDet_conf det_conf;
	struct sLvw_conf lvw_conf;

	//// Functions ////

	void update_led();
	void set_reference_depth_image();
	void set_capture_video_image(int num);
	uint32_t compare_depth_frame_to_reference_depth_frame();
	bool init_num_detection();
	bool delete_detections();

	static void *detection_thread_helper(void *context);
	static void *liveview_thread_helper(void *context);

	void *detection(void);
	void *liveview(void);

	template <typename T>
	int change_det_status(enum enumDet_conf, T value);
};

extern int pipe_fd[2];

#endif /* CALARM_H_ */
