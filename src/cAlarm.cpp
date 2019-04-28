/**
  * @file cAlarma.cpp
  * @author Alejandro Solozabal
  */

#include "cAlarm.h"

cAlarm::cAlarm()
{
	// Frame pointers
	reff_depth_frame		= NULL;
	depth_frame				= NULL;
	diff_depth_frame		= NULL;
	temp_depth_frame		= NULL;
	liveview_frame			= NULL;
	liveview_jpeg			= NULL;
	liveview_buffer_out 	= NULL;
	for(int i = 0; i< NUM_DETECTIONS_FRAMES ; i++)
		video_frames[i]		= NULL;

	reff_depth_timestamp	= 0;
	depth_timestamp			= 0;
	video_timestamp			= 0;
	liveview_timestamp		= 0;

	temp_depth_frame_timestamp = 0;

	// Threads ID
	detection_thread		= 0;
	liveview_thread			= 0;

	// Running flags
	detection_running		= false;
	liveview_running		= false;

	// Detection config
	det_conf.is_active		= false;
	det_conf.threshold		= DETECTION_THRESHOLD;
	det_conf.tolerance		= DEPTH_CHANGE_TOLERANCE;
	det_conf.det_num_shots	= NUM_DETECTIONS_FRAMES;
	det_conf.frame_interval	= FRAME_INTERVAL_US/1000000;
	det_conf.curr_det_num	= 0;

	// LiveView config
	lvw_conf.is_active		= false;

	// Syslog initialization
	openlog ("kinect_alarm::cAlarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
}

cAlarm::~cAlarm()
{
	// TODO Auto-generated destructor stub
}

int cAlarm::init()
{
	// Parse config file
	if(parse_conf_file(&det_conf,&lvw_conf,CONFIG_PATH))
	{
		// Generate new config file with default values
		write_conf_file(det_conf,lvw_conf,CONFIG_PATH);
	}

	// Redis db initialization
	if(init_redis_db())
	{
		LOG(LOG_ERR,"Fallo en la inicializacion de Redis\n");
		kinect.deinit();
		return -1;
	}

	// Initialize redis vars
	if(init_vars_redis())
	{
		LOG(LOG_ERR,"Fallo en la inicializacion de variables de Redis\n");
		kinect.deinit();
		return -1;
	}

	// SQLite initialization
	if(init_sqlite_db())
	{
		LOG(LOG_ERR,"Fallo en la inicializacion de SQLite\n");
		kinect.deinit();
		return -1;
	}

	// Gen detection table on SQLite
	if(create_det_table_sqlite_db())
	{
		LOG(LOG_ERR,"Error: generation detection table on SQLite\n");
		kinect.deinit();
		return -1;
	}

	// Init number of entries in detection table
	int num_det;
	number_entries_det_table_sqlite_db(&num_det);

	// Kinect initialization
	if(kinect.init())
	{
		LOG(LOG_ERR,"Fallo en la inicializacion de kinect\n");
		kinect.deinit();
		return -1;
	}

	// Adjust kinect's tilt
	if(kinect.change_tilt(0))
	{
		LOG(LOG_ERR,"Fallo al cambiar la inclinacion de kinect\n");
		kinect.deinit();
		return -1;
	}

	// Update kinect led
	update_led();

	// Create base directory to save detection images
	create_dir((char *)DETECTION_PATH);

/*
	if(init_num_detection()) //TODO: delete function
	{
		LOG(LOG_ERR,"Num detection exedded\n");
		return -1;
	}
*/
	// Memory allocation for detection frames ponters
	reff_depth_frame	= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	depth_frame			= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	diff_depth_frame	= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));

	temp_depth_frame	= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));

	if (pthread_mutex_init(&diff_depth_frame_lock, NULL) != 0)
	{
		return -1;
	}

	if(!reff_depth_frame || !depth_frame || !diff_depth_frame)
	{
		LOG(LOG_ERR,"Memory allocation for frames\n");
		return -1;
	}

	for(int i = 0; i< NUM_DETECTIONS_FRAMES ; i++)
	{
		video_frames[i] = (uint16_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t));
		if(!video_frames[i])
		{
			LOG(LOG_ERR,"Memory allocation for frames\n");
			return -1;
		}
	}

	// Memory allocation for liveview frames pointers
	liveview_frame = (uint16_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t));
	liveview_jpeg = (uint8_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint8_t)*2);
	liveview_buffer_out = (uint8_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint8_t)*2);

	// Apply XML config TODO
	if(det_conf.is_active)
		start_detection();
	if(lvw_conf.is_active)
		start_liveview();

	return 0;
}

int cAlarm::deinit()
{
	if(detection_running)
	{
		detection_running = false;
		pthread_join(detection_thread,NULL);
		update_led();
	}
	if(liveview_running)
	{
		liveview_running = false;
		pthread_join(liveview_thread,NULL);
		update_led();
	}
	if(kinect.is_kinect_running())
		kinect.stop();
	kinect.deinit();

	// Deinit Redis
	deinit_redis_db();

	// Deinit SQLite
	deinit_sqlite_db();

	// Free memory pointers
	free(reff_depth_frame);
	free(depth_frame);
	free(diff_depth_frame);

	free(temp_depth_frame);

	for(int i = 0; i< NUM_DETECTIONS_FRAMES ; i++)
		free(video_frames[i]);

	free(liveview_frame);
	free(liveview_jpeg);
	free(liveview_buffer_out);

	return 0;
}

int cAlarm::start_detection()
{
	if(!kinect.is_kinect_running())
		kinect.start();

	if(!detection_running)
	{
		detection_running = true;
		change_det_status(DET_ACTIVE,true);
		redis_set_int((char *) "det_status", 1);
		update_led();
		pthread_create(&detection_thread, 0, detection_thread_helper, this);
		return 0;
	}
	return 1;
}

int cAlarm::stop_detection()
{
	if(detection_running)
	{
		detection_running = false;
		change_det_status(DET_ACTIVE,false);
		redis_set_int((char *) "det_status", 0);
		pthread_join(detection_thread,NULL);
		update_led();

		if(!detection_running && !liveview_running)
			kinect.stop();

		return 0;
	}
	return 1;
}

int cAlarm::start_liveview()
{
	if(!kinect.is_kinect_running())
		kinect.start();

	if(!liveview_running)
	{
		liveview_running = true;
		change_lvw_status(LVW_ACTIVE,true);
		redis_set_int((char *) "lvw_status", 1);
		update_led();
		pthread_create(&liveview_thread, 0, liveview_thread_helper, this);
		return 0;
	}

	return 0;
}

int cAlarm::stop_liveview()
{
	if(liveview_running)
	{
		liveview_running = false;
		change_lvw_status(LVW_ACTIVE,false);
		redis_set_int((char *) "lvw_status", 0);
		pthread_join(liveview_thread,NULL);
		update_led();

		if(!detection_running && !liveview_running)
			kinect.stop();
	}
	return 0;
}

uint32_t cAlarm::compare_depth_frame_to_reference_depth_frame()
{
	uint32_t cont = 0;
	static uint32_t max_cont = 0;
	pthread_mutex_lock(&diff_depth_frame_lock);
	for(int i = 0; i <(DEPTH_WIDTH*DEPTH_HEIGHT);i++)
	{

		if(depth_frame[i] == 0x07FF || reff_depth_frame[i] == 0x07FF)
			diff_depth_frame[i] = 0;
		else
		{
			diff_depth_frame[i] =abs(depth_frame[i] - reff_depth_frame[i]);
			if(diff_depth_frame[i] > DEPTH_CHANGE_TOLERANCE)
				cont++;
			else
				diff_depth_frame[i] = 0;
		}
	}
	pthread_mutex_unlock(&diff_depth_frame_lock);
	if(cont > max_cont)
		max_cont = cont;
	//printf("DIFF %u\t MAX DIFF %u\n",cont,max_cont);
	return cont;
}

bool cAlarm::init_num_detection()
{
	char path[PATH_MAX];

	det_conf.curr_det_num = 0;

	for(int i = 0; i < MAX_NUM_DETECTIONS; i++)
	{
		sprintf(path,"%s/%d",DETECTION_PATH,det_conf.curr_det_num);
		if(!check_dir_exist(path))
			return false;
		else
			det_conf.curr_det_num++;
	}

	return true;
}

int cAlarm::get_diff_depth_frame(uint16_t *diff_depth_frame, uint32_t *timestamp)
{

	kinect.get_depth_frame(temp_depth_frame,&temp_depth_frame_timestamp);

	return 0;
}


void *cAlarm::detection(void)
{
	// Variable initialization
	reff_depth_timestamp	= 0;
	depth_timestamp			= 0;
	video_timestamp			= 0;
	time_t t;

	while(detection_running)
	{
		update_led();

		// Get Reference frame

		if(kinect.get_depth_frame(reff_depth_frame,&reff_depth_timestamp))
		{
			LOG(LOG_ERR,"Failed to capture depth frame\n");
			kinect.deinit();
			return 0;
		}
		LOG(LOG_INFO,"Reference depth frame captured\n");

		// Detection depth changes

		int diff_cont = 0;
		do
		{
			// get depth image to compare
			if(kinect.get_depth_frame(depth_frame,&depth_timestamp))
			{
				LOG(LOG_ERR,"Failed to capture depth frame\n");
				kinect.deinit();
				return 0;
			}
			diff_cont = compare_depth_frame_to_reference_depth_frame();
		}while(diff_cont < DETECTION_THRESHOLD && detection_running);


		// Detection occurs
		if(detection_running)
		{
			time(&t);

			// Publish event
			char message[100];
			sprintf(message, "newdet %d",det_conf.curr_det_num);
			redis_publish("kinectalarm_event",message);

			kinect.change_led_color(LED_RED);
			LOG(LOG_ALERT,"DETECTION\n");

			for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
			{
				if(kinect.get_video_frame(video_frames[i],&video_timestamp))
				{
					LOG(LOG_ERR,"Failed to capture video frame\n");
					kinect.deinit();
					return 0;
				}
				LOG(LOG_INFO,"Video Frame captured\n");
				if(i != (NUM_DETECTIONS_FRAMES-1))
					usleep(FRAME_INTERVAL_US);
			}
/*
			char temp[PATH_MAX];
			sprintf(temp,"%s/%d",DETECTION_PATH,det_conf.curr_det_num);
			create_dir(temp);
*/
			char filepath[PATH_MAX];
			sprintf(filepath,"%s/%u_%s",DETECTION_PATH,t,"ref_depth.jpeg");
			if(save_video_frame_to_jpeg(reff_depth_frame,filepath))
				LOG(LOG_ERR,"Error saving depth frame\n");

			sprintf(filepath,"%s/%u_%s",DETECTION_PATH,t,"diff.jpeg");
			if(save_video_frame_to_jpeg(diff_depth_frame,filepath))
				LOG(LOG_ERR,"Error saving depth frame\n");

			for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
			{
				sprintf(filepath,"%s/%u_capture_%d.jpeg",DETECTION_PATH,t,i);
				if(save_video_frame_to_jpeg(video_frames[i],filepath))
					LOG(LOG_ERR,"Error saving video frame\n");
			}

			//save_video_frames_to_gif(video_frames,NUM_DETECTIONS_FRAMES,(1/0.2),(char *)"asd");


			// Update SQLite db
			insert_entry_det_table_sqlite_db(det_conf.curr_det_num,t,5,filepath); //TODO check if it fails

			// Update Redis db
			redis_set_int((char *) "det_numdet", det_conf.curr_det_num); //TODO check if it fails

			change_det_status(CURR_DET_NUM,det_conf.curr_det_num+1);
		}
	}
	return 0;
}


void *cAlarm::detection_thread_helper(void *context)
{
	return ((cAlarm *)context)->detection();
}

/*
void *cAlarm::liveview(void)
{

	struct timespec sleep_time = {0,41666666};
	struct timespec wakeup_time;
	struct timespec current_time;
	struct timespec sleep_remain_time = {0,0};
	
	init_video();
	start_video();
	
	clock_gettime(CLOCK_MONOTONIC, &wakeup_time);

	while(liveview_running)
	{
		wakeup_time = timeAdd(wakeup_time, sleep_time);
		
		kinect.get_video_frame(liveview_frame);
		send_frame(liveview_frame);
		
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		sleep_remain_time = timeSub(wakeup_time,current_time);
		printf("usec to sleep:%d\n",sleep_remain_time.tv_nsec/1000);
		
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wakeup_time, NULL);
	}

	stop_video();
	deinit_video();
	return 0;
}
*/

void *cAlarm::liveview(void)
{
	unsigned int size = 0;

	// Variable initialization
	liveview_timestamp				= 0;
	struct sBase64encode_context c	={0};


	init_base64encode(&c);

	while(liveview_running)
	{
		// Get new video frame and convert it to jpeg
		kinect.get_video_frame(liveview_frame,&liveview_timestamp);

		// Convert to jpeg
		save_video_frame_to_jpeg_inmemory(liveview_frame, liveview_jpeg,&size);

		// Convert to base64
		char *base64_encoded = base64encode(&c, liveview_jpeg, size);

		// Publish in redis channel
		redis_publish("liveview", base64_encoded);

		usleep(100000);//TODO parameter
	}

	deinit_base64encode(&c);

	return 0;
}

void *cAlarm::liveview_thread_helper(void *context)
{
	return ((cAlarm *)context)->liveview();
}

void cAlarm::update_led()
{
	if(liveview_running && detection_running)
		kinect.change_led_color((freenect_led_options)4);
	else if(detection_running)
		kinect.change_led_color(LED_YELLOW);
	else if(liveview_running)
		kinect.change_led_color((freenect_led_options)5);
	else
		kinect.change_led_color(LED_OFF);
}

bool cAlarm::is_detection_running()
{
	if(detection_running)
		return true;
	else
		return false;
}

bool cAlarm::is_liveview_running()
{
	if(liveview_running)
		return true;
	else
		return false;
}

int cAlarm::get_num_detections()
{
	return det_conf.curr_det_num;
}

bool cAlarm::delete_detections()
{
	//TODO
	return true;
}

int cAlarm::reset_detection()
{
	int det_was_running = detection_running;
	if(det_was_running)
		stop_detection();
	delete_all_entries_det_table_sqlite_db();

	if(det_was_running)
		start_detection();
	return 0;
}

template <typename T>
int cAlarm::change_det_status(enum enumDet_conf conf_name, T value)
{
	switch(conf_name)
	{
		case DET_ACTIVE:
			det_conf.is_active = value;
			break;
		case THRESHOLD:
			det_conf.threshold = value;
			break;
		case TOLERANCE:
			det_conf.tolerance = value;
			break;
		case DET_NUM_SHOTS:
			det_conf.det_num_shots = value;
			break;
		case FRAME_INTERVAL:
			det_conf.frame_interval = value;
			break;
		case CURR_DET_NUM:
			det_conf.curr_det_num = value;
			break;
	}
	write_conf_file(det_conf,lvw_conf,CONFIG_PATH);
	return 0;
}

template <typename T>
int cAlarm::change_lvw_status(enum enumLvw_conf conf_name, T value)
{
	switch(conf_name)
	{
		case LVW_ACTIVE:
			lvw_conf.is_active = value;
			break;
	}
	write_conf_file(det_conf,lvw_conf,CONFIG_PATH);
	return 0;
}

int cAlarm::init_vars_redis()
{
	if(redis_set_int((char *) "det_status", det_conf.is_active))
		return -1;
	if(redis_set_int((char *) "lvw_status", lvw_conf.is_active))
		return -1;
	if(redis_set_int((char *) "det_numdet", det_conf.curr_det_num-1))
		return -1;

	return 0;
}
