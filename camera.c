/*
 * File    : camera.c
 *
 * Author  : com0716
 *
 * Date    : 2017-11-03-10.14
 *
 * CopyRight by Werewolf
 */
 
#include "camera.h"
#include "bmp.h"
 
camera_t* camera_open(const char *device_path, uint32_t width, uint32_t height)
{
	int fd = open(device_path, O_RDWR | O_NONBLOCK);
	if (fd < 0)
	{
		return NULL;
	}
	
	camera_t* p_camera = (camera_t *)malloc(sizeof(camera_t));
	if (NULL == p_camera)
	{
		close(fd);
		return NULL;
	}
	
	memset(p_camera, 0, sizeof(camera_t));
	p_camera->fd = fd;
	p_camera->width = width;
	p_camera->height = height;
	p_camera->buffer_count = 0;
	p_camera->buffers = NULL;
	p_camera->head.start = NULL;
	p_camera->head.length = 0;
	
	return p_camera;	
}

void camera_init(camera_t* p_camera)
{
	struct v4l2_capability cap;
	
	memset(&cap, 0, sizeof(cap));
	if (ioctl(p_camera->fd, VIDIOC_QUERYCAP, &cap) >= 0)
	{
		printf("version:%d, cap:%d\n", cap.version, cap.capabilities);
		if (! (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
		{
			printf("[ERROR] camera_init: no campture\n");
			return ;
		}
		
		if (! (cap.capabilities & V4L2_CAP_STREAMING))
		{
			printf("[ERROR] camera_init: no streaming\n");
			return ;
		}		
	}
	
	struct v4l2_cropcap cropcap;
	memset(&cropcap, 0, sizeof(cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(p_camera->fd, VIDIOC_CROPCAP, &cropcap) >= 0)
	{
		struct v4l2_crop crop;
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;
		
		ioctl(p_camera->fd, VIDIOC_S_CROP, &crop);
	}
	
	struct v4l2_format format;
	memset(&format, 0, sizeof(format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = p_camera->width;
	format.fmt.pix.height = p_camera->height;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	format.fmt.pix.field = V4L2_FIELD_NONE;
	format.fmt.pix.width = p_camera->width;
	ioctl(p_camera->fd, VIDIOC_S_FMT, &format);
	
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ioctl(p_camera->fd, VIDIOC_REQBUFS, &req);
	p_camera->buffer_count = req.count;
	p_camera->buffers = calloc(req.count, sizeof(buffer_t));
	
	size_t i = 0;
	size_t buf_max = 0;
	struct v4l2_buffer buffer;
	for (i = 0; i < p_camera->buffer_count; ++i)
	{
		memset(&buffer, 0, sizeof(buffer));
		
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		
		ioctl(p_camera->fd, VIDIOC_QUERYBUF, &buffer);
		if (buffer.length > buf_max)
			buf_max = buffer.length;
		
		printf("count:%d, length:%d\n", i, buffer.length);
		p_camera->buffers[i].length = buffer.length;
		p_camera->buffers[i].start = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, p_camera->fd, buffer.m.offset);
		if (NULL == p_camera->buffers[i].start)
		{
			perror("mmap error");
			return ;
		}
	}
	
	p_camera->head.start = (uint8_t *)malloc(buf_max);
	
	return ;
}

void camera_start(camera_t* p_camera)
{
	size_t i = 0;
	struct v4l2_buffer buffer;
	for (i = 0; i < p_camera->buffer_count;  ++i)
	{
		memset(&buffer, 0, sizeof(buffer));
		
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		
		ioctl(p_camera->fd, VIDIOC_QBUF, &buffer);
	}
	
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(p_camera->fd, VIDIOC_STREAMON, &type);
	
	return ;
}

int camera_capture(camera_t* p_camera)
{
	struct v4l2_buffer buffer;
	
	memset(&buffer, 0, sizeof(buffer));
		
	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	
	ioctl(p_camera->fd, VIDIOC_DQBUF, &buffer);
	
	memcpy(p_camera->head.start, p_camera->buffers[buffer.index].start, buffer.bytesused);
	p_camera->head.length = buffer.bytesused;
	
	ioctl(p_camera->fd, VIDIOC_QBUF, &buffer);
	
	return 1;
}

int camera_frame(camera_t* p_camera, struct timeval timeout)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(p_camera->fd, &fds);
	
	int nready = select(p_camera->fd + 1, &fds, NULL, NULL, &timeout);
	if (nready < 0)
	{
		perror("[ERROR] camera_frame");
		return nready;
	}
	
	if (nready == 0)
	{
		return nready;
	}
	
	return camera_capture(p_camera);
}

void camera_stop(camera_t* p_camera)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(p_camera->fd, VIDIOC_STREAMOFF, &type);
	
	return ;
}

void camera_release(camera_t* p_camera)
{
	size_t i = 0;
	for (i = 0; i < p_camera->buffer_count;  ++i)
	{
		munmap(p_camera->buffers[i].start, p_camera->buffers[i].length);
	}
	
	free(p_camera->buffers);
	p_camera->buffer_count = 0;
	p_camera->buffers = NULL;
	
	free(p_camera->head.start);
	p_camera->head.length = 0;
	p_camera->head.start = NULL;
	
	return ;
}

void camera_close(camera_t* p_camera)
{
	close(p_camera->fd);
	free(p_camera);
}


static inline int minmax(int min, int v, int max)
{
	return (v<min)?min:((max<v)?max:v);
}

static uint8_t* yuyv2rgb(uint8_t* yuyv, uint32_t width, uint32_t height)
{
	int i = 0, j = 0;
	int32_t y0 = 0, u = 0, y1 = 0, v = 0;
	int index = 0;
	uint8_t* rgb = calloc(width*height*3, sizeof(uint8_t));
	
	for (i = 0; i < height; ++i)
	{
		for (j = 0; j < width; j += 2)
		{
			index = i * width + j;
			y0 = yuyv[index*2 + 0] << 8;
			v = yuyv[index*2 + 1] - 128;
			y1 = yuyv[index*2 + 2] << 8;
			u = yuyv[index*2 + 3] - 128;
			
			rgb[index*3 + 0] = minmax(0, (y0 + 359*v)>>8, 255);
			rgb[index*3 + 1] = minmax(0, (y0 + 88*v - 183*u)>>8, 255);
			rgb[index*3 + 2] = minmax(0, (y0 + 454*u)>>8, 255);
			rgb[index*3 + 3] = minmax(0, (y1 + 359*v)>>8, 255);
			rgb[index*3 + 4] = minmax(0, (y1 + 88*v - 183*u)>>8, 255);
			rgb[index*3 + 5] = minmax(0, (y1 + 454*u)>>8, 255);
		}
	}
	
	return rgb;
}

void camera_create_bmp(camera_t* p_camera, const char *bmp_path)
{
	uint8_t* rgb = yuyv2rgb(p_camera->head.start, p_camera->width, p_camera->height);
	
	int fd = open(bmp_path, O_RDWR | O_CREAT, 0755);
	if (fd < 0)
	{
		perror("[ERROR] camera_create_bmp open image file");
		free(rgb);
		return ;
	}
	
	if (bmp_create(fd, rgb, p_camera->width, p_camera->height) < 0)
	{
		printf("[ERROR] camera_create_bmp create bmp");
	}	
	
	close(fd);
	
	free(rgb);
	
	return ;
}