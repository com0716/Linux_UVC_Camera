/*
 * File    : camera.h
 *
 * Author  : com0716
 *
 * Date    : 2017-11-03-10.14
 *
 * CopyRight by Werewolf
 */
#ifndef CAMERA_H_
#define CAMERA_H_

#include "includes.h"

#define UVC_DEVICE_PATH "/dev/video0"

typedef struct {
	uint8_t *start;
	size_t length;
}buffer_t;

typedef struct {
	int fd;
	uint32_t width;
	uint32_t height;
	size_t buffer_count;
	buffer_t *buffers;
	buffer_t head;
}camera_t;

camera_t* camera_open(const char *device_path, uint32_t width, uint32_t height);

void camera_init(camera_t* p_camera);

void camera_start(camera_t* p_camera);

int camera_capture(camera_t* p_camera);

int camera_frame(camera_t* p_camera, struct timeval timeout);

void camera_stop(camera_t* p_camera);

void camera_release(camera_t* p_camera);

void camera_close(camera_t* p_camera);

void camera_create_bmp(camera_t* p_camera, const char *bmp_path);

#endif // CAMERA_H_
