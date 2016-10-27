#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <map>
#include <vector>
#include "strutil.h"
#include "LibRaw-0.17.2/libraw/libraw.h"


char * _loadRAW_m(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string>& info, std::string params, char * icc_m, unsigned  *icclength)
{

	int w, h, c, b;
	LibRaw RawProcessor;
	char * img;
	char imgdata[4096];

	std::map<std::string,std::string> p = parseparams(params);

	//for (std::map<std::string,std::string>::iterator it=p.begin(); it!=p.end(); ++it)
	//	std::cout << it->first << ": " << it->second << std::endl;
	//printf("\n");
	

#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
//#define OUT RawProcessor.imgdata.params

	RawProcessor.imgdata.params.shot_select = 0;
	RawProcessor.imgdata.params.use_camera_wb = 1;
	RawProcessor.imgdata.params.output_color = 1;	//sRGB
	RawProcessor.imgdata.params.user_qual = 3;	//AHD

	RawProcessor.imgdata.params.output_bps = 16;

	RawProcessor.imgdata.params.gamm[0] = 1/1.0;   //1/2.222;
	RawProcessor.imgdata.params.gamm[1] = 1.0;     //4.5;



	//# colorspace:raw|srgb|adobe|wide|prophoto|xyz - Output color space, default=srgb
	if (p.find("colorspace") != p.end()) {
		if (p["colorspace"].compare("raw") == 0) 
			RawProcessor.imgdata.params.output_color = 0;
		if (p["colorspace"].compare("srgb") == 0) 
			RawProcessor.imgdata.params.output_color = 1;
		if (p["colorspace"].compare("adobe") == 0) 
			RawProcessor.imgdata.params.output_color = 2;
		if (p["colorspace"].compare("wide") == 0) 
			RawProcessor.imgdata.params.output_color = 3;
		if (p["colorspace"].compare("prophoto") == 0) 
			RawProcessor.imgdata.params.output_color = 4;
		if (p["colorspace"].compare("xyz") == 0) 
			RawProcessor.imgdata.params.output_color = 5;
	}

	//# demosaic:linear|vng|ppg|ahd|dcb - Demosaic algorithm, default=ahd
	if (p.find("demosaic") != p.end()) {
		if (p["demosaic"].compare("linear") == 0) 
			RawProcessor.imgdata.params.user_qual = 0;
		if (p["demosaic"].compare("vng") == 0) 
			RawProcessor.imgdata.params.user_qual = 1;
		if (p["demosaic"].compare("ppg") == 0) 
			RawProcessor.imgdata.params.user_qual = 2;
		if (p["demosaic"].compare("ahd") == 0) 
			RawProcessor.imgdata.params.user_qual = 3;
		if (p["demosaic"].compare("dcb") == 0) 
			RawProcessor.imgdata.params.user_qual = 4;
	}

	//# bps:8|16 - bits per sample, default=16
	if (p.find("bps") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = atoi(p["bps"].c_str());

	//# gamma:bt709|srgb|linear - Apply one of the gamma presets
	if (p.find("gamma") != p.end()) {
		if (p["gamma"].compare("bt709") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/2.222;
			RawProcessor.imgdata.params.gamm[1] = 4.5;
		}
		if (p["gamma"].compare("srgb") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/2.4;
			RawProcessor.imgdata.params.gamm[1] = 12.92;
		}
		if (p["gamma"].compare("linear") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/1.0;
			RawProcessor.imgdata.params.gamm[1] = 1.0;
		}
	}

	//# gammaval:2.222 - Set specific gamma value, default=2.222
	if (p.find("gammaval") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = atof(p["gammaval"].c_str());

	//# gammatoe:4.5 - Set specific gamma toe, default=4.5
	if (p.find("gammatoe") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = atof(p["gammatoe"].c_str());


	//# autobright - Use dcraw automatic brightness
	if (p.find("autobright") != p.end()) 
		RawProcessor.imgdata.params.no_auto_bright = 0; 
	else
		RawProcessor.imgdata.params.no_auto_bright = 1; 


	RawProcessor.open_file(filename);
	RawProcessor.unpack();
	RawProcessor.dcraw_process();
	RawProcessor.get_mem_image_format(&w, &h, &c, &b);
	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;

	img = (char *)malloc(w*h*c*(b/8));
	
	libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image();
	memcpy(img, image->data, image->data_size);
	free(image);

	icc_m = NULL;
	*icclength = 0;

	info["ISOSpeedRatings"] = tostr(P2.iso_speed);  
	info["ExposureTime"] = tostr(P2.shutter);  
	info["FNumber"] = tostr(P2.aperture);  
	info["FocalLength"] = tostr(P2.focal_len);  
	info["ImageDescription"] = P2.desc;  
	info["Artist"] = P2.artist; 
	info["Make"] = P1.make;  
	info["Model"] = P1.model;  

	time_t rawtime = P2.timestamp;
	struct tm * timeinfo;
	char buffer [80];
	timeinfo = localtime (&rawtime);
	strftime (buffer,80,"%Y:%m:%d %H:%M:%S",timeinfo);
	info["DateTime"] = buffer;  


	RawProcessor.recycle();

	return img;

}


