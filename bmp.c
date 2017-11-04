/*
 * File    : bmp.c
 *
 * Author  : com0716
 *
 * Date    : 2017-11-03-10.14
 *
 * CopyRight by Werewolf
 */
#include "bmp.h"

static void swap(uint8_t* bytes, uint32_t index, uint32_t re_index)
{
	bytes[index] = bytes[index] ^ bytes[re_index];
	bytes[re_index] = bytes[index] ^ bytes[re_index];
	bytes[index] = bytes[index] ^ bytes[re_index];
}

int bmp_create(int fd, uint8_t* rgb, uint32_t width, uint32_t height)
{
	bitmap_file_header_t header;
	bitmap_info_header_t info;
	uint32_t data_len = 3*width*height;

	memset(&header, 0, sizeof(header));
	header.bf_type = 0x4D42;
	header.bf_size[0] = (BITMAP_HEADER_LENGTH + data_len) & 0xFFFF;
	header.bf_size[1] = ((BITMAP_HEADER_LENGTH + data_len) >> 16) & 0xFFFF;
	header.bf_offbits[0] = (BITMAP_HEADER_LENGTH) & 0xFFFF;
	header.bf_offbits[1] = ((BITMAP_HEADER_LENGTH) >> 16) & 0xFFFF;
	
	memset(&info, 0, sizeof(info));
	info.bi_size = BITMAP_INFO_HEADER_LENGTH;
	info.bi_width = width;
	info.bi_height = height;
	info.bi_planes = 0x01;
	info.bi_bitcount = 0x18;
	info.bi_compression = 0;	
	info.bi_image_size = data_len;
	info.bi_xpels_per_meter = 0;
	info.bi_ypels_per_meter = 0;
	info.bi_used_clr = 0;
	info.bi_important_clr = 0;
	
	//文件头写入
	write(fd, &header, sizeof(header));
	//文件信息写入
	write(fd, &info, sizeof(info));
	
	//主体数据写入
	uint32_t i = 0, j = 0;
	for (i = 0; i < height; ++i)
	{
		for (j = 0; j < width/2;  ++j)
		{
			uint32_t index = (i*width + j) * 3;
			uint32_t re_index = (i*width + (width - j - 1)) * 3;
			
			swap(rgb, index + 0, re_index + 0);
			swap(rgb, index + 1, re_index + 1);
			swap(rgb, index + 2, re_index + 2);		
		}
	}
	write(fd, rgb, data_len);
	
	return 0;
}

