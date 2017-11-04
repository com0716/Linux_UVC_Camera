/*
 * File    : main.c
 *
 * Author  : com0716
 *
 * Date    : 2017-11-03-10.14
 *
 * CopyRight by Werewolf
 */
#include "includes.h"
#include "camera.h"

int main(int argc, char *argv[])
{
	int i = 0;
	const char *bmp_path = "output.bmp";
	camera_t* p_camera = NULL;
	struct timeval timeout;
	
	//如果文件不存在
	if (access(UVC_DEVICE_PATH, F_OK) != 0)
	{
		perror("[ERROR] can not find the usb camera device");
		return -1;
	}
	
	p_camera = camera_open(UVC_DEVICE_PATH, 640, 480);
	if (NULL == p_camera)
	{
		perror("[ERROR] can not open the usb camera device");
		return -1;
	}
	
	camera_init(p_camera);
	camera_start(p_camera);	
	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	//跳过5个frame，然后采集，给camera一点启动的时间
	for (i = 0; i < 10; ++i)
	{
		camera_frame(p_camera, timeout);
	}
	camera_frame(p_camera, timeout);	
	
	camera_create_bmp(p_camera, bmp_path);
	
	camera_stop(p_camera);
	camera_release(p_camera);
	camera_close(p_camera);	
	
	return 0;
}