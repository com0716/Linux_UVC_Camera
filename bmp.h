/*
 * File    : bmp.h
 *
 * Author  : com0716
 *
 * Date    : 2017-11-03-10.14
 *
 * CopyRight by Werewolf
 */
#ifndef BMP_H_
#define BMP_H_

#include "includes.h"

#define BITMAP_FILE_HEADER_LENGTH 14
#define BITMAP_INFO_HEADER_LENGTH 40
#define BITMAP_HEADER_LENGTH 54

typedef struct {
	uint16_t bf_type;
	uint16_t bf_size[2];
	uint16_t bf_reversed[2];
	uint16_t bf_offbits[2];
}bitmap_file_header_t;

typedef struct {
	uint32_t bi_size;
	uint32_t bi_width;
	uint32_t bi_height;
	uint16_t bi_planes;
	uint16_t bi_bitcount;
	uint32_t bi_compression;
	uint32_t bi_image_size;
	uint32_t bi_xpels_per_meter;
	uint32_t bi_ypels_per_meter;
	uint32_t bi_used_clr;
	uint32_t bi_important_clr;
}bitmap_info_header_t;

int bmp_create(int fd, uint8_t* rgb, uint32_t width, uint32_t height);

#endif // BMP_H_
