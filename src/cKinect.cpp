/**
  * @file cKinect.cpp
  * @author Alejandro Solozabal
  */

#include "cKinect.h"


//Static cKinect variables declaration

uint16_t* cKinect::temp_depth_frame_raw;
uint16_t* cKinect::temp_video_frame_raw;

uint32_t cKinect::temp_depth_frame_timestamp;
uint32_t cKinect::temp_video_frame_timestamp;

pthread_mutex_t cKinect::depth_lock;
pthread_mutex_t cKinect::video_lock;

pthread_cond_t  cKinect::depth_ready;
pthread_cond_t  cKinect::video_ready;

cKinect::cKinect()
{
	// Members initialization
	is_kinect_initialize	= false;
	kinect_ctx				= NULL;
	kinect_dev				= NULL;
	running					= false;
	process_event_thread	= 0;
}

cKinect::~cKinect()
{
	// TODO Auto-generated destructor stub
}


int cKinect::init()
{
	int ret = 0;

	// Check if it's already initialize
	if(is_kinect_initialize)
		return 0;

	//Mutex init
	pthread_mutex_init(&cKinect::depth_lock, NULL);
	pthread_mutex_init(&cKinect::video_lock, NULL);

	//new frame Mutex init
	pthread_cond_init(&cKinect::depth_ready, NULL);
	pthread_cond_init(&cKinect::video_ready, NULL);

	// Library freenect init
	ret = freenect_init(&kinect_ctx, NULL);
	if (ret < 0)
		return -1;

	// Set log level
	freenect_set_log_level(kinect_ctx, FREENECT_LOG_FATAL);

	// Select subdevices
	freenect_select_subdevices(kinect_ctx, (freenect_device_flags) (FREENECT_DEVICE_MOTOR | FREENECT_DEVICE_CAMERA));

	// Find out how many devices are connected.
	int num_devices = ret = freenect_num_devices(kinect_ctx);
	if (ret < 0)
	{
		LOG(LOG_ERR,"Error searching for Kinect devices\n");
		freenect_shutdown(kinect_ctx);
		return -1;
	}

	else if (num_devices == 0)
	{
		LOG(LOG_ERR,"No Kinect device found\n");
		freenect_shutdown(kinect_ctx);
		return -1;
	}

	// Open the first device.
	ret = freenect_open_device(kinect_ctx, &kinect_dev, 0);
	if (ret < 0)
	{
		freenect_shutdown(kinect_ctx);
		return -1;
	}

	// Configure depth and video mode
	ret = freenect_set_depth_mode(kinect_dev, freenect_find_depth_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_DEPTH_11BIT));
	if (ret < 0)
	{
		freenect_shutdown(kinect_ctx);
		return -1;
	}

	ret = freenect_set_video_mode(kinect_dev, freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_IR_10BIT));
	if (ret < 0)
	{
		freenect_shutdown(kinect_ctx);
		return -1;
	}

	// Set frame callbacks.
	freenect_set_depth_callback(kinect_dev, depth_cb);
	freenect_set_video_callback(kinect_dev, video_cb);

	//Malloc for frames
	temp_depth_frame_raw = (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	temp_video_frame_raw = (uint16_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t));

	// Initialize frame time-stamps
	cKinect::temp_depth_frame_timestamp = 0;
	cKinect::temp_video_frame_timestamp = 0;

	// Set kinect init flag to true
	is_kinect_initialize = true;

	return 0;
}

int cKinect::deinit()
{
	LOG(LOG_INFO,"Shutting down kinect\n");

	// Stop everything and shutdown.
	if(kinect_dev)
	{
		freenect_close_device(kinect_dev);
		freenect_shutdown(kinect_ctx);
	}

	// Free memory pointers
	free(temp_depth_frame_raw);
	free(temp_video_frame_raw);

	// Initialize flag to false
	is_kinect_initialize = false;

	return 0;
}

int cKinect::start()
{
	// Initialize frame time-stamps
	cKinect::temp_depth_frame_timestamp = 0;
	cKinect::temp_video_frame_timestamp = 0;

	running = true;
	freenect_start_video(kinect_dev);
	freenect_start_depth(kinect_dev);
	pthread_create(&process_event_thread, 0, kinect_process_events_helper, this);
	return 0;
}

int cKinect::stop()
{
	running = false;
	pthread_join(process_event_thread,NULL);
	if(kinect_dev)
	{
		freenect_stop_depth(kinect_dev);
		freenect_stop_video(kinect_dev);
	}
	return 0;
}

int cKinect::get_depth_frame(uint16_t *depth_frame, uint32_t *timestamp)
{
	pthread_mutex_lock(&cKinect::depth_lock);

	// Compare the given timestamp with the current, if it's the same must wait to the next frame
	if(*timestamp == cKinect::temp_depth_frame_timestamp)
		pthread_cond_wait(&cKinect::depth_ready, &cKinect::depth_lock);

	memcpy (depth_frame, cKinect::temp_depth_frame_raw, (DEPTH_WIDTH*DEPTH_HEIGHT)*sizeof(uint16_t));
	*timestamp = cKinect::temp_depth_frame_timestamp;

	pthread_mutex_unlock(&cKinect::depth_lock);
	return 0;
}

int cKinect::get_video_frame(uint16_t *video_frame, uint32_t *timestamp)
{

	pthread_mutex_lock(&cKinect::video_lock);

	// Compare the given timestamp with the current, if it's the same must wait to the next frame
	if(*timestamp == cKinect::temp_video_frame_timestamp)
		pthread_cond_wait(&cKinect::video_ready, &cKinect::video_lock);

	memcpy (video_frame, cKinect::temp_video_frame_raw, (VIDEO_WIDTH*VIDEO_HEIGHT)*sizeof(uint16_t));
	*timestamp = cKinect::temp_video_frame_timestamp;

	pthread_mutex_unlock(&cKinect::video_lock);
	return 0;
}


void cKinect::depth_cb(freenect_device* dev, void* data, uint32_t timestamp)
{
	pthread_mutex_lock(&cKinect::depth_lock);
	memcpy (cKinect::temp_depth_frame_raw, data, (DEPTH_WIDTH*DEPTH_HEIGHT)*sizeof(uint16_t));
	cKinect::temp_depth_frame_timestamp = timestamp;
	pthread_cond_broadcast(&cKinect::depth_ready);//pthread_cond_signal
	pthread_mutex_unlock(&cKinect::depth_lock);
	return;
}

void cKinect::video_cb(freenect_device* dev, void* data, uint32_t timestamp)
{
	pthread_mutex_lock(&cKinect::video_lock);
	memcpy (cKinect::temp_video_frame_raw, data, (VIDEO_WIDTH*VIDEO_HEIGHT)*sizeof(uint16_t));
	cKinect::temp_video_frame_timestamp = timestamp;
	pthread_cond_broadcast(&cKinect::video_ready);
	pthread_mutex_unlock(&cKinect::video_lock);
	return;
}


bool cKinect::change_tilt(double tilt_angle)
{
	freenect_raw_tilt_state* state;
	freenect_update_tilt_state(kinect_dev);
	int num_tries = 0;

	// Send the desire tilt
	if(freenect_set_tilt_degs(kinect_dev, tilt_angle))
		return true;

	// Loop waiting
	do
	{
		if(num_tries++ > MAX_TILT_WAIT)
			return true;

		freenect_update_tilt_state(kinect_dev);
		state = freenect_get_tilt_state(kinect_dev);
		sleep(1);
	}
	while((state->tilt_status != TILT_STATUS_STOPPED) && (state->tilt_status != TILT_STATUS_LIMIT));

	return false;
}

void *cKinect::kinect_process_events(void)
{
	// Loop processing packets from kinect
	do
	{
		if(freenect_process_events(kinect_ctx) < 0)
			return 0;

	}while(running);
	return 0;
}

void *cKinect::kinect_process_events_helper(void *context)
{
	return ((cKinect *)context)->kinect_process_events();
}

void cKinect::change_led_color(freenect_led_options color)
{
	freenect_set_led(kinect_dev,color);
}

bool cKinect::is_kinect_running()
{
	if(running)
		return true;
	else
		return false;
}
