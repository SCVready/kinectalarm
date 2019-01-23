/**
  * @file cAlarma.cpp
  * @author Alejandro Solozabal
  */

#include "cAlarma.h"

cAlarma::cAlarma()
{
	// Frame pointers
	reff_depth_frame	= NULL;
	depth_frame			= NULL;
	diff_depth_frame	= NULL;

	// Detection Thread ID
	detection_thread	= 0;

	// Running flags
	detection_running	= false;
	liveview_running	= false;

	// Detection config
	det_conf.is_active		= false;
	det_conf.threshold		= DETECTION_THRESHOLD;
	det_conf.tolerance		= DEPTH_CHANGE_TOLERANCE;
	det_conf.det_num_shots	= NUM_DETECTIONS_FRAMES;
	det_conf.frame_interval	= NUM_DETECTIONS_FRAMES;
	det_conf.curr_det_num	= 0;

	// LiveView config
	lvw_conf.is_active		= false;

	// Syslog initialization
	openlog ("kinect_alarm::cAlarm", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
}

cAlarma::~cAlarma()
{
	// TODO Auto-generated destructor stub
}

int cAlarma::init()
{
	// Parse config file
	if(parse_conf_file(&det_conf,CONF_PATH))
	{
		// Generate new config file with default values
		write_conf_file(det_conf,CONF_PATH);
	}

	// Kinect initialization
	if(kinect.init())
	{
		LOG(LOG_ERR,"Fallo en la inicializacion de kinect");
		kinect.deinit();
		return -1;
	}

	// Adjust kinect's tilt
	if(kinect.change_tilt(-25))
	{
		LOG(LOG_ERR,"Fallo al cambiar la inclinacion de kinect");
		kinect.deinit();
		return -1;
	}

	// Update kinect led
	update_led();

	// Create base directory to save detection images
	create_dir((char *)PATH);

/*	//TODO
	if(init_num_detection())
	{
		LOG(LOG_ERR,"Num detection exedded\n");
		return -1;
	}
*/
	// Memory allocation for frames
	reff_depth_frame	= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	depth_frame			= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	diff_depth_frame	= (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));

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

	// Apply XML config TODO
	if(det_conf.is_active)
		start_detection();

	return 0;
}

int cAlarma::deinit()
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
		//TODO
		update_led();
	}
	if(kinect.is_kinect_running())
		kinect.stop();
	kinect.deinit();
	return 0;
}

int cAlarma::start_detection()
{
	if(!kinect.is_kinect_running())
		kinect.start();

	if(!detection_running)
	{
		detection_running = true;
		change_det_status(IS_ACTIVE,true);
		update_led();
		pthread_create(&detection_thread, 0, detection_thread_helper, this);
		return 0;
	}
	return 1;
}

int cAlarma::stop_detection()
{
	if(detection_running)
	{
		detection_running = false;
		change_det_status(IS_ACTIVE,false);
		pthread_join(detection_thread,NULL);
		update_led();

		if(!detection_running && !liveview_running)
			kinect.stop();

		return 0;
	}
	return 1;
}

int cAlarma::start_liveview()
{
	// TODO
	return 0;
}

int cAlarma::stop_liveview()
{
	// TODO
	return 0;
}

uint32_t cAlarma::compare_depth_frame_to_reference_depth_image()
{
	uint32_t cont = 0;
	static uint32_t max_cont = 0;
	for(int i = 0; i <(DEPTH_WIDTH*DEPTH_HEIGHT);i++)
	{

		if(depth_frame[i] == 0x07FF || reff_depth_frame[i] == 0x07FF)
			diff_depth_frame[i] = 0;
		else
			diff_depth_frame[i] =abs(depth_frame[i] - reff_depth_frame[i]);

		if(diff_depth_frame[i] > DEPTH_CHANGE_TOLERANCE)
			cont++;
	}
	if(cont > max_cont)
		max_cont = cont;
	//printf("DIFF %u\t MAX DIFF %u\n",cont,max_cont);
	return cont;
}

bool cAlarma::init_num_detection()
{
	char path[PATH_MAX];

	det_conf.curr_det_num = 0;

	for(int i = 0; i < MAX_NUM_DETECTIONS; i++)
	{
		sprintf(path,"%s/%d",PATH,det_conf.curr_det_num);
		if(!check_dir_exist(path))
			return false;
		else
			det_conf.curr_det_num++;
	}

	return true;
}

bool cAlarma::save_depth_frame_to_bmp(uint16_t* depth_frame,char *filename)
{
	FIBITMAP *depth_bitmap;
	char filepath[PATH_MAX];
	bool retval = false;
	sprintf(filepath,"%s/%d/%s",PATH,det_conf.curr_det_num,filename);

	depth_bitmap = FreeImage_ConvertFromRawBits((BYTE *) depth_frame, DEPTH_WIDTH, DEPTH_HEIGHT, DEPTH_WIDTH*2, 16, 0x00FF, 0x00FF, 0x00FF, FALSE);
	FreeImage_FlipVertical(depth_bitmap);
	if(!FreeImage_Save(FIF_BMP, depth_bitmap, filepath, 0))
		retval = true;

	FreeImage_Unload(depth_bitmap);
	return retval;
}

bool cAlarma::save_video_frame_to_bmp(uint16_t* video_frame,char *filename)
{
	FIBITMAP *video_bitmap;
	uint8_t bmap[VIDEO_WIDTH*VIDEO_HEIGHT*3];

	char filepath[PATH_MAX];
	bool retval = false;
	sprintf(filepath,"%s/%d/%s",PATH,det_conf.curr_det_num,filename);

	//TODO TEMPORAL
	for(int i = 0;i <  VIDEO_WIDTH * VIDEO_HEIGHT; i++)
	{
		bmap[3*i]   = video_frame[i];
		bmap[3*i+1] = video_frame[i];
		bmap[3*i+2] = video_frame[i];
	}

	video_bitmap = FreeImage_ConvertFromRawBits((BYTE *) bmap, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH*3, 24, 0xFF0000,  0x00FF00, 0x0000FF, TRUE);
	FreeImage_AdjustBrightness(video_bitmap, 80); //TODO not working
	if(!FreeImage_Save(FIF_JPEG, video_bitmap, filepath, 0))
		retval = true;

	FreeImage_Unload(video_bitmap);
	return retval;
}

bool cAlarma::save_video_frames_to_gif(uint16_t** video_frames_array, int num_frames, float frame_interval, char *filename)
{
	//TODO
	FIBITMAP *video_bitmap[num_frames];
	FIBITMAP *video_bitmap_rescale[num_frames];
	FIBITMAP *video_bitmap_tmp[num_frames];
	uint8_t bmap_array[VIDEO_WIDTH*VIDEO_HEIGHT*3];
	bool retval = false;

	for(int i = 0; i < num_frames; i++)
	{
		for(int j = 0; j <  VIDEO_WIDTH * VIDEO_HEIGHT; j++)
		{
			bmap_array[3*j]   = *(video_frames_array[i]+j);
			bmap_array[3*j+1] = *(video_frames_array[i]+j);
			bmap_array[3*j+2] = *(video_frames_array[i]+j);

		}

		video_bitmap[i] = FreeImage_ConvertFromRawBits((BYTE *) bmap_array, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH*3, 24, 0xFF0000,  0x00FF00, 0x0000FF, TRUE);
		FreeImage_AdjustBrightness(video_bitmap[i], 80); //TODO not working
		video_bitmap_rescale[i] = FreeImage_Rescale(video_bitmap[i], VIDEO_WIDTH, VIDEO_HEIGHT, FILTER_CATMULLROM);
		video_bitmap_tmp[i] = FreeImage_ConvertTo8Bits(video_bitmap_rescale[i]);
	}

	// assume we have an array of dibs which are already 8bpp and all the same size,
	// and some float called fps for frames per second

	FIMULTIBITMAP *multi = FreeImage_OpenMultiBitmap(FIF_GIF, "output.gif", TRUE, FALSE);
	float fps = 1/frame_interval;
	DWORD dwFrameTime = (DWORD)((1000.0f / fps) + 0.5f);
	for(int i = 0; i < num_frames; i++ )
	{
		// clear any animation metadata used by this dib as we’ll adding our own ones
		FreeImage_SetMetadata(FIMD_ANIMATION, video_bitmap_tmp[i], NULL, NULL);
		// add animation tags to dib[i]
		FITAG *tag = FreeImage_CreateTag();
		if(tag)
		{
			FreeImage_SetTagKey(tag, "FrameTime");
			FreeImage_SetTagType(tag, FIDT_LONG);
			FreeImage_SetTagCount(tag, 1);
			FreeImage_SetTagLength(tag, 4);
			FreeImage_SetTagValue(tag, &dwFrameTime);
			FreeImage_SetMetadata(FIMD_ANIMATION, video_bitmap_tmp[i], FreeImage_GetTagKey(tag), tag);
			FreeImage_DeleteTag(tag);
		}
		FreeImage_AppendPage(multi, video_bitmap[i]);
	}
	retval = FreeImage_CloseMultiBitmap(multi,0);

	for(int i = 0; i<num_frames ; i++)
	{
		FreeImage_Unload(video_bitmap[i]);
		FreeImage_Unload(video_bitmap_tmp[i]);
	}


	return retval;
}

void *cAlarma::detection(void)
{
	while(det_conf.curr_det_num < MAX_NUM_DETECTIONS && detection_running)
	{
		update_led();

		// Get Reference frame

		if(kinect.get_depth_frame(reff_depth_frame))
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
			if(kinect.get_depth_frame(depth_frame))
			{
				LOG(LOG_ERR,"Failed to capture depth frame\n");
				kinect.deinit();
				return 0;
			}
			diff_cont = compare_depth_frame_to_reference_depth_image();
		}while(diff_cont < DETECTION_THRESHOLD && detection_running);


		// Detection occurs

		if(detection_running)
		{
			kinect.change_led_color(LED_RED);
			LOG(LOG_ALERT,"DETECTION\n");

			for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
			{
				if(kinect.get_video_frame(video_frames[i]))
				{
					LOG(LOG_ERR,"Failed to capture video frame\n");
					kinect.deinit();
					return 0;
				}
				LOG(LOG_INFO,"Video Frame captured\n");
				if(i != (NUM_DETECTIONS_FRAMES-1))
					usleep(FRAME_INTERVAL_US);
			}

			char temp[PATH_MAX];
			sprintf(temp,"%s/%d",PATH,det_conf.curr_det_num);
			create_dir(temp);


			if(save_depth_frame_to_bmp(reff_depth_frame,(char *)"ref_depth.bmp"))
				LOG(LOG_ERR,"Error saving depth frame\n");
			if(save_depth_frame_to_bmp(diff_depth_frame,(char *)"diff.bmp"))
				LOG(LOG_ERR,"Error saving depth frame\n");

			for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
			{
				sprintf(temp,"capture_%d.jpeg",i);
				if(save_video_frame_to_bmp(video_frames[i],temp))
					LOG(LOG_ERR,"Error saving video frame\n");
			}

			//save_video_frames_to_gif(video_frames,NUM_DETECTIONS_FRAMES,(1/0.2),(char *)"asd");

			//det_conf.curr_det_num++;
			change_det_status(CURR_DET_NUM,det_conf.curr_det_num+1);
		}
	}
	return 0;
}


void *cAlarma::detection_thread_helper(void *context)
{
	return ((cAlarma *)context)->detection();
}

void cAlarma::update_led()
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

bool cAlarma::is_detection_running()
{
	if(detection_running)
		return true;
	else
		return false;
}

bool cAlarma::is_liveview_running()
{
	if(liveview_running)
		return true;
	else
		return false;
}

int cAlarma::get_num_detections()
{
	return det_conf.curr_det_num;
}

bool cAlarma::delete_detections()
{

	return true;
}

int cAlarma::reset_detection()
{
	int det_was_running = detection_running;
	if(det_was_running)
		stop_detection();
	change_det_status(CURR_DET_NUM,0);
	if(det_was_running)
		start_detection();
	return 0;
}

template <typename T>
int cAlarma::change_det_status(enum enumDet_conf conf_name, T value)
{
	switch(conf_name)
	{
		case IS_ACTIVE:
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
	write_conf_file(det_conf,CONF_PATH);
	return 0;
}

