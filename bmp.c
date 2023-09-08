#include "bmp.h"
#include <stdio.h>
#include <stdlib.h>

#define WRITE_2BYTES(val) fprintf(img->outfile, "%c%c", (val), (val)>>8)
#define WRITE_3BYTES(val) fprintf(img->outfile, "%c%c%c", (val), (val)>>8, (val)>>16) 
#define WRITE_4BYTES(val) fprintf(img->outfile, "%c%c%c%c", (val), (val)>>8, (val)>>16, (val)>>24)

#define OFFSET2(val) (val), (val)>>8
#define OFFSET4(val) (val), (val)>>8, (val)>>16, (val)>>24

void bmp_init(bmp* img, const char* outfile_name, unsigned int width, unsigned int height){
	if( !(img->outfile = fopen(outfile_name, "w")) ){
		fprintf(stderr, "Couldn't open file\n");
		exit(1);
	}
	img->width = width;
	img->height = height;
}

void bmp_write_header(bmp* img){
	// offset for bit data 26
	
	const unsigned int size = 26 + img->width*img->height*3; //assuming 24bits (3bytes) per pixel	
	const unsigned int offset = 26;
	char buff[27];
	sprintf(buff, "BM%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
		OFFSET4(size),
		OFFSET4('\0'),
		OFFSET4(offset),
		OFFSET4(12U),
		OFFSET2(img->width),
		OFFSET2(img->height),
		OFFSET2(1U),
		OFFSET2(24U)
	);

	fwrite(buff, 1, 26, img->outfile);
}

void bmp_free(bmp* img){
	fclose(img->outfile);
}

void bmp_buff_init(bmp_buff* buff, bmp* img, char* pixbuff){
	buff->img = img;
	buff->pixbuff = pixbuff;
	buff->size = 0;
}

void bmp_buff_reset(bmp_buff* buff){
	buff->size = 0;
}

void bmp_buff_push_pixel(bmp_buff* buff, unsigned int pix){
	buff->pixbuff[buff->size++] = (char)(pix>>0);
	buff->pixbuff[buff->size++] = (char)(pix>>8);
	buff->pixbuff[buff->size++] = (char)(pix>>16);
}

void bmp_buff_write(bmp_buff* buff){
	fwrite(buff->pixbuff, 1, buff->size, buff->img->outfile);
}

