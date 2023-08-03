#ifndef BMP_H
#define BMP_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	FILE* outfile;
	unsigned int width;
	unsigned int height;
} bmp;

typedef struct {
	bmp* img;
	char* pixbuff;
	size_t size;
} bmp_buff;

// bpm funcs
void bmp_init(bmp* img, const char* outfile_name, unsigned int width, unsigned int height);
void bmp_write_header(bmp* img);
void bmp_free(bmp* img);

// bmp_buff funcs
void bmp_buff_init(bmp_buff* buff, bmp* img, char* pixbuff);
void bmp_buff_reset(bmp_buff* buff);
void bmp_buff_push_pixel(bmp_buff* buff, unsigned int pix);
void bmp_buff_write(bmp_buff* buff);

#ifdef __cplusplus
}
#endif

#endif

