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
#define OUT RawProcessor.imgdata.params

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

	info["iso"] = tostr(P2.iso_speed);
	info["shutter"] = tostr(P2.shutter);
	info["aperture"] = tostr(P2.aperture);
	info["focallength"] = tostr(P2.focal_len);
	info["shottime"] = tostr(P2.timestamp);
	info["description"] = P2.desc;
	info["artist"] = P2.artist;
	info["make"] = P1.make;
	info["model"] = P1.model;

	RawProcessor.recycle();

	return img;

}


