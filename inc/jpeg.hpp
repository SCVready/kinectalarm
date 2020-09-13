/**
 * @author Alejandro Solozabal
 *
 * @file jpeg.hpp
 *
 */

#ifndef JPEG_H_
#define JPEG_H_

/*******************************************************************
 * Includes
 *******************************************************************/
#include <string.h>
#include <vector>
#include <FreeImage.h>

/*******************************************************************
 * Funtion declaration
 *******************************************************************/
bool save_depth_frame_to_jpeg(uint16_t* depth_frame,char *filepath);
bool save_video_frame_to_jpeg(uint16_t* video_frame,char *filepath, int32_t brightness, int32_t contrast);
bool save_video_frame_to_jpeg_inmemory(uint16_t* video_frame, std::vector<uint8_t>& video_jpeg, int32_t brightness, int32_t contrast);
bool save_video_frames_to_gif(uint16_t** video_frames_array, int num_frames, float frame_interval, char *filename);

#endif /* JPEG_H_ */
