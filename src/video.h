/*
 * video.h
 *
 *  Created on: May 13, 2019
 *      Author: scvready
 */

#ifndef VIDEO_H_
#define VIDEO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

int encode_video_from_frames(char *filename,uint16_t **video_frames,int num_frames,int height,int width,int fps);

#endif /* VIDEO_H_ */
