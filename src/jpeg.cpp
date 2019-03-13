/*
 * jpeg.cpp
 *
 *  Created on: Mar 13, 2019
 *      Author: scvready
 */

#include "jpeg.h"
#include "global_parameters.h"



bool save_depth_frame_to_jpeg(uint16_t* depth_frame,char *filepath)
{
	FIBITMAP *depth_bitmap;

	bool retval = false;

	depth_bitmap = FreeImage_ConvertFromRawBits((BYTE *) depth_frame, DEPTH_WIDTH, DEPTH_HEIGHT, DEPTH_WIDTH*2, 16, 0x00FF, 0x00FF, 0x00FF, FALSE);
	FreeImage_FlipVertical(depth_bitmap);
	if(!FreeImage_Save(FIF_BMP, depth_bitmap, filepath, 0))
		retval = true;

	FreeImage_Unload(depth_bitmap);
	return retval;
}

bool save_video_frame_to_jpeg(uint16_t* video_frame,char *filepath)
{
	FIBITMAP *video_bitmap;
	uint8_t bmap[VIDEO_WIDTH*VIDEO_HEIGHT*3];

	bool retval = false;

	//TODO TEMPORAL
	for(int i = 0;i <  VIDEO_WIDTH * VIDEO_HEIGHT; i++)
	{
		bmap[3*i]   = video_frame[i];
		bmap[3*i+1] = video_frame[i];
		bmap[3*i+2] = video_frame[i];
	}

	video_bitmap = FreeImage_ConvertFromRawBits((BYTE *) bmap, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH*3, 24, 0xFF0000,  0x00FF00, 0x0000FF, TRUE);
	FreeImage_AdjustBrightness(video_bitmap, 80); //TODO Parametric brightness
	if(!FreeImage_Save(FIF_JPEG, video_bitmap, filepath, 0))
		retval = true;

	FreeImage_Unload(video_bitmap);
	return retval;
}

bool save_video_frame_to_jpeg_inmemory(uint16_t* video_frame, uint8_t* video_jpeg, uint32_t *size_bytes)
{
	FIBITMAP *video_bitmap;
	uint8_t bmap[VIDEO_WIDTH*VIDEO_HEIGHT*3];
	bool retval = false;

	FIMEMORY *hmem = NULL;
	hmem = FreeImage_OpenMemory();

	// get the buffer from the memory stream
	BYTE *mem_buffer = NULL;
	DWORD size_in_bytes = 0;


	//TODO TEMPORAL
	for(int i = 0;i <  VIDEO_WIDTH * VIDEO_HEIGHT; i++)
	{
		bmap[3*i]   = video_frame[i];
		bmap[3*i+1] = video_frame[i];
		bmap[3*i+2] = video_frame[i];
	}

	video_bitmap = FreeImage_ConvertFromRawBits((BYTE *) bmap, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_WIDTH*3, 24, 0xFF0000,  0x00FF00, 0x0000FF, TRUE);
	FreeImage_AdjustBrightness(video_bitmap, 80); //TODO Parametric brightness

	if(!FreeImage_SaveToMemory(FIF_JPEG, video_bitmap, hmem, 0))
		retval = true;

	FreeImage_AcquireMemory(hmem, &mem_buffer, &size_in_bytes);
	memcpy(video_jpeg,mem_buffer,size_in_bytes);
	*size_bytes = size_in_bytes;

	FreeImage_CloseMemory(hmem);
	FreeImage_Unload(video_bitmap);
	return retval;
}

bool save_video_frames_to_gif(uint16_t** video_frames_array, int num_frames, float frame_interval, char *filepath)
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
		// clear any animation metadata used by this dib as we�ll adding our own ones
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



