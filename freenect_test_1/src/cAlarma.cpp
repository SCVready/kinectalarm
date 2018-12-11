/*
 * cAlarma.cpp
 *
 *  Created on: 18 ago. 2018
 *      Author: asolo
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "FreeImage.h"

#include "cKinect.h"
#include "cAlarma.h"

cAlarma::cAlarma()
{
	num_detections 		= 0;
	reff_depth_frame	= NULL;
	depth_frame			= NULL;
	diff_depth_frame	= NULL;
	running				= true;
	detection_thread	= 0;
}

cAlarma::~cAlarma()
{
	// TODO Auto-generated destructor stub
}

int cAlarma::init()
{
	// Inicializacion de kinect
	if(kinect.init())
	{
		printf("Fallo en la inicializacion de kinect");
		kinect.deinit();
		return -1;
	}
	// Ajuste de la inclinacion de kinect
	if(kinect.change_tilt(-25))
	{
		printf("Fallo al cambiar la inclinacion de kinect");
		kinect.deinit();
		return -1;
	}

	// Create base dir
	create_dir((char *)LOCAL_PATH);

	if(init_num_detection())
	{
		printf("Num detection exedded\n");
		return true;
	}

	reff_depth_frame = (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	depth_frame = (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));
	diff_depth_frame = (uint16_t*) malloc (DEPTH_WIDTH * DEPTH_HEIGHT * sizeof(uint16_t));

	for(int i = 0; i< NUM_DETECTIONS_FRAMES ; i++)
		video_frames[i] = (uint16_t*) malloc (VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint16_t));

	return 0;
}
int cAlarma::deinit()
{
	kinect.deinit();
	return 0;
}

int cAlarma::run()
{
	running = true;
	kinect.run();
	pthread_create(&detection_thread, 0, detection_thread_helper, this);
	return 0;
}

int cAlarma::stop()
{
	running = false;
	kinect.change_led_color(LED_GREEN);
	kinect.stop();
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

	num_detections = 0;

	for(int i = 0; i < MAX_NUM_DETECTIONS; i++)
	{
		sprintf(path,"%s/%d",LOCAL_PATH,num_detections);
		if(!check_dir_exist(path))
			return false;
		else
			num_detections++;
	}

	return true;
}

bool cAlarma::save_depth_frame_to_bmp(uint16_t* depth_frame,char *filename)
{
	FIBITMAP *depth_bitmap;
	char filepath[PATH_MAX];
	sprintf(filepath,"%s/%d/%s",LOCAL_PATH,num_detections,filename);

	depth_bitmap = FreeImage_ConvertFromRawBits((BYTE *) depth_frame, DEPTH_WIDTH, DEPTH_HEIGHT, DEPTH_WIDTH*2, 16, 0x00FF, 0x00FF, 0x00FF, FALSE);
	FreeImage_FlipVertical(depth_bitmap);
	if(!FreeImage_Save(FIF_BMP, depth_bitmap, filepath, 0))
		return true;
	return false;
}
bool cAlarma::save_video_frame_to_bmp(uint16_t* video_frame,char *filename)
{
	FIBITMAP *video_bitmap;
	char filepath[PATH_MAX];
	sprintf(filepath,"%s/%d/%s",LOCAL_PATH,num_detections,filename);

	video_bitmap = FreeImage_ConvertFromRawBits((BYTE *) video_frame, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH*2, 16, 0x03FF, 0x03FF, 0x03FF, FALSE);
	FreeImage_FlipVertical(video_bitmap);
	if(!FreeImage_Save(FIF_BMP, video_bitmap, filepath, 0))
		return true;
	return false;
}
void *cAlarma::detection(void)
{
	while(num_detections < MAX_NUM_DETECTIONS && running)
	{
		kinect.change_led_color(LED_YELLOW);
		// Get Reference frame
		if(kinect.get_depth_frame(reff_depth_frame))
		{
			printf("Fallo al capturar un frame de depth");
			kinect.deinit();
			return NULL;
		}
		printf("Reference depth frame get\n");
		// Deteccion
		int diff_cont = 0;
		do
		{
			// get depth image to compare
			if(kinect.get_depth_frame(depth_frame))
			{
				printf("Fallo al capturar un frame de depth");
				kinect.deinit();
				return NULL;
			}
			diff_cont = compare_depth_frame_to_reference_depth_image();
		}while(diff_cont < DETECTION_THRESHOLD && running);


		if(running)
		{
			kinect.change_led_color(LED_RED);
			printf("Intrusion happen\n");

			for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
			{
				if(kinect.get_video_frame(video_frames[i]))
				{
					printf("Fallo al capturar un frame de video");
					kinect.deinit();
					return NULL;
				}
				printf("Video frame captured\n");
				if(i != (NUM_DETECTIONS_FRAMES-1))
					usleep(200000);
			}

			char temp[PATH_MAX];
			sprintf(temp,"%s/%d",LOCAL_PATH,num_detections);
			create_dir(temp);


			if(save_depth_frame_to_bmp(reff_depth_frame,(char *)"ref_depth.bmp"))
				printf("Error al guardar el archivo\n");
			if(save_depth_frame_to_bmp(diff_depth_frame,(char *)"diff.bmp"))
				printf("Error al guardar el archivo\n");

			for(int i = 0; i < NUM_DETECTIONS_FRAMES; i++)
			{
				sprintf(temp,"capture_%d.bmp",i);
				if(save_video_frame_to_bmp(video_frames[i],temp))
					printf("Error al guardar el archivo\n");
			}

			num_detections++;
		}
	}
	return NULL;
}


void *cAlarma::detection_thread_helper(void *context)
{
	return ((cAlarma *)context)->detection();
}
