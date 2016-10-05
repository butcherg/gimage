#include <stdio.h>
#include <stdlib.h>
#include "LibRaw-0.17.2/libraw/libraw.h"

char * _loadRAW(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits)
{

	int w, h, c, b;
	LibRaw RawProcessor;
	char * img;

	RawProcessor.imgdata.params.shot_select = 0;
	RawProcessor.imgdata.params.use_camera_wb = 1;
	RawProcessor.imgdata.params.output_color = 1;	//sRGB, hardcoded for now
	RawProcessor.imgdata.params.user_qual = 3;	//AHD

	RawProcessor.imgdata.params.output_bps = 8;
	RawProcessor.imgdata.params.gamm[0] = 1/2.222;	//1;
	RawProcessor.imgdata.params.gamm[1] = 4.5;	//1;
	RawProcessor.imgdata.params.no_auto_bright = 0; //1;

	RawProcessor.open_file(filename);
	RawProcessor.unpack();
	RawProcessor.dcraw_process();
	RawProcessor.get_mem_image_format(&w, &h, &c, &b);
	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;

	img = (char *)malloc(w*h*c);
	
	libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image();
	memcpy(img, image->data, image->data_size);
	free(image);
	
	RawProcessor.recycle();

	return img;

/*
	unsigned row_stride;
	libraw_data_t *iprc = libraw_init(0);
	char * img;
	int ret = libraw_open_file(iprc,filename);

	printf("Processing %s (%s %s)\n",filename,iprc->idata.make,iprc->idata.model);

	ret = libraw_unpack(iprc);

	ret = libraw_dcraw_process(iprc);

	libraw_processed_image_t *pimg = libraw_dcraw_make_mem_image(iprc,NULL);
	*width = pimg->width;
	*height = pimg->height;
	*numcolors = pimg->colors;
	*numbits = pimg->bits;

	row_stride = *width * *numcolors * *numbits;
	img = (char *)malloc(*height * row_stride);
	memcpy(img, pimg->data, pimg->data_size);
	free(pimg);
	libraw_close(iprc);

	imagedata = img;
	return img;
*/	
}

