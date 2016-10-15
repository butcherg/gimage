#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <map>
#include "strutil.h"
#include "LibRaw-0.17.2/libraw/libraw.h"


char * _loadRAW_m(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string>& info, char * icc_m, unsigned  *icclength)
{

	int w, h, c, b;
	LibRaw RawProcessor;
	char * img;
	char imgdata[4096];

#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
//#define OUT RawProcessor.imgdata.params

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

	icc_m = NULL;
	*icclength = 0;

	info["ISOSpeedRatings"] = tostr(P2.iso_speed);  //ISOSpeedRatings, int16u
	info["ExposureTime"] = tostr(P2.shutter);  //ExposureTime, rational64u
	info["FNumbe"] = tostr(P2.aperture);  //FNumber, rational64u
	info["FocalLength"] = tostr(P2.focal_len);  //FocalLength, rational64u
	info["DateTime"] = tostr(P2.timestamp);  //DateTime, //DateTimeOriginal, string
	info["ImageDescription"] = P2.desc;  //ImageDescription, string
	info["Artist"] = P2.artist;  //Artist, string
	info["Make"] = P1.make;  //Make, string
	info["Model"] = P1.model;  //Model, string

	RawProcessor.recycle();

	return img;

}


