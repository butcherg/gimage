#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <map>
#include <vector>
#include <gimage/strutil.h>
#include <libraw/libraw.h>
#include <lcms2.h>
#include <gimage/gimage.h>

const char * librawVersion()
{
	return LibRaw::version();
}

bool _checkRAW(const char *filename)
{
	LibRaw RawProcessor;
	if (RawProcessor.open_file(filename) == LIBRAW_SUCCESS) return true;
	return false;
}


bool _loadRAWInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info)
{
	int w, h, c, b;
	LibRaw RawProcessor;

#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
// #define OUT RawProcessor.imgdata.params

	if (RawProcessor.open_file(filename) == LIBRAW_SUCCESS) {
		RawProcessor.unpack();

		info["ISOSpeedRatings"] = tostr(P2.iso_speed);  
		info["ExposureTime"] = tostr(P2.shutter);  
		info["FNumber"] = tostr(P2.aperture);  
		info["FocalLength"] = tostr(P2.focal_len);  
		info["ImageDescription"] = P2.desc;  
		info["Artist"] = P2.artist; 
		info["Make"] = P1.make;  
		info["Model"] = P1.model;  
		info["Orientation"] = tostr(S.flip);

		time_t rawtime = P2.timestamp;
		struct tm * timeinfo;
		char buffer [80];
		timeinfo = localtime (&rawtime);
		strftime (buffer,80,"%Y:%m:%d %H:%M:%S",timeinfo);
		info["DateTime"] = buffer;  
		RawProcessor.recycle();
		return true;
	}
	else {
		RawProcessor.recycle();
		return false;
	}
}

char * _loadRAW(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params="",
			char ** icc_m=NULL, 
			unsigned  *icclength=0)
{

	int w, h, c, b;
	LibRaw RawProcessor;
	char * img;
	char imgdata[4096];
	
	int rawdata = 0;

	std::map<std::string,std::string> p = parseparams(params);

	//for (std::map<std::string,std::string>::iterator it=p.begin(); it!=p.end(); ++it)
	//	std::cout << it->first << ": -" << it->second << "-" << std::endl;
	//printf("\n");
	
#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
// #define OUT RawProcessor.imgdata.params

	RawProcessor.imgdata.params.shot_select = 0;
	RawProcessor.imgdata.params.use_camera_wb = 1;
	RawProcessor.imgdata.params.output_color = 1;	//sRGB
	RawProcessor.imgdata.params.user_qual = 3;	//AHD

	RawProcessor.imgdata.params.output_bps = 16;

	RawProcessor.imgdata.params.gamm[0] = 1/2.4;  //1/1.0
	RawProcessor.imgdata.params.gamm[1] = 12.92; //1.0

	//#
	//# rawdata=0|1 - Preempts all other parameters, if 1 loads unprocessed raw data as a one-color 16-bit grayscale.  Default=0.
	//#
	if (p.find("rawdata") != p.end()) 
		rawdata = atoi(p["rawdata"].c_str());
	
	//#
	//# output_color=0|1|2|3|4|5 - Output color space, default=1 (srgb)
	//# colorspace=raw|srgb|adobe|wide|prophoto|xyz - Alias of output_color, with mnemonic values. default=srgb
	//#
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

	if (p.find("output_color") != p.end()) 
		RawProcessor.imgdata.params.output_color = atoi(p["output_color"].c_str());

	//#
	//# user_qual=0|1|2|3|4 - Demosaic algorithm, default=3 (ahd)
	//# demosaic=linear|vng|ppg|ahd|dcb - Alias of user_qual, with mnemonic values. default=ahd
	//#
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
	if (p.find("user_qual") != p.end()) RawProcessor.imgdata.params.user_qual = atoi(p["user_qual"].c_str());

	//#
	//# output_bps=8|16 - bits per sample, default=16
	//#
	if (p.find("output_bps") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = atoi(p["bps"].c_str());

	//# Gamma:  three ways to set it:
	//#       1. gamma=preset
	//#       2. gammaval=2.222;gammatoe=4.5
	//#       3. gamm=2.222,4.5
	//#
	//#    gamma=bt709|srgb|linear|prophoto - Gamma presets
	//#
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
		if (p["gamma"].compare("prophoto") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/1.8;
			RawProcessor.imgdata.params.gamm[1] = 0.0;
		}
	}

	//#
	//#    gammaval=2.222 - Set specific power gamma value, overrides preset, default=1.0 (linear)
	//#    gammatoe=4.5 - Set specific gamma toe, overrides preset, default=1.0 (linear)
	// gamm1=4.5 - LIbRaw alias
	//#
	if (p.find("gammaval") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = 1.0/atof(p["gammaval"].c_str());
	if (p.find("gammatoe") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = atof(p["gammatoe"].c_str());

	//#    gamm=2.222,4.5 - Set gamm[0] and gamm[1]
	//#
	if (p.find("gamm") != p.end()) {
		std::vector<std::string> g = split(p["gamm"],",");
		RawProcessor.imgdata.params.gamm[0] = 1.0/atof(g[0].c_str());
		RawProcessor.imgdata.params.gamm[1] = atof(g[1].c_str());
	}

	//#
	//# autobright=0|1 - Use dcraw automatic brightness (note the difference from no_auto_bright)
	//#
	if (p.find("autobright") != p.end()) 
		RawProcessor.imgdata.params.no_auto_bright = 0; 
	else
		RawProcessor.imgdata.params.no_auto_bright = 1; 

	//#
	//# greybox=x,y,w,h - coordinates of a rectangle used for white balance 
	//#
	if (p.find("greybox") != p.end()) {
		std::vector<std::string> g = split(p["greybox"],",");
		if (g.size() == 4) {
			RawProcessor.imgdata.params.greybox[0] = atoi(g[0].c_str());
			RawProcessor.imgdata.params.greybox[1] = atoi(g[1].c_str());
			RawProcessor.imgdata.params.greybox[2] = atoi(g[2].c_str());
			RawProcessor.imgdata.params.greybox[3] = atoi(g[3].c_str());
		}
	}

	//#
	//# cropbox=x,y,w,h - coordinates of a rectangle used for crop 
	//#
	if (p.find("cropbox") != p.end()) {
		std::vector<std::string> c = split(p["cropbox"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.cropbox[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.cropbox[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.cropbox[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.cropbox[3] = atoi(c[3].c_str());
		}
	}

	//#
	//# aber=rmult,gmult - chromatic aberration correction 
	//#
	if (p.find("aber") != p.end()) {
		std::vector<std::string> c = split(p["aber"],",");
		if (c.size() == 2) {
			RawProcessor.imgdata.params.aber[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.aber[2] = atoi(c[1].c_str());
		}
	}

	//#
	//# user_mul=mul0,mul1,mul2,mul3 - user white balance multipliers, r,g,b,g
	//#
	if (p.find("user_mul") != p.end()) {
		std::vector<std::string> c = split(p["user_mul"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.user_mul[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.user_mul[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.user_mul[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.user_mul[3] = atoi(c[3].c_str());
		}
	}

	//#
	//# shot_select=n - select image number for processing
	//#
	if (p.find("shot_select") != p.end()) 
		RawProcessor.imgdata.params.shot_select = atoi(p["shot_select"].c_str());

	//#
	//# bright=1.0 - brighten image, default=1.0
	//#
	if (p.find("bright") != p.end()) 
		RawProcessor.imgdata.params.bright = atof(p["bright"].c_str());

	//#
	//# threshold=1.0 - wavelet denoising threshold
	//#
	if (p.find("threshold") != p.end()) 
		RawProcessor.imgdata.params.threshold = atof(p["threshold"].c_str());

	//#
	//# half_size=0|1 - output image in half-size
	//#
	if (p.find("half_size") != p.end()) 
		RawProcessor.imgdata.params.half_size = atoi(p["half_size"].c_str());

	//#
	//# four_color_rgb=0|1 - separate interpolations for the two green components
	//#
	if (p.find("four_color_rgb") != p.end()) 
		RawProcessor.imgdata.params.four_color_rgb = atoi(p["four_color_rgb"].c_str());

	//#
	//# highlight=0|1|2|3+ - deal with image highlights, clip=0, 1=unclip, 2=blend, 3+=rebuild
	//#
	if (p.find("highlight") != p.end()) 
		RawProcessor.imgdata.params.highlight = atoi(p["highlight"].c_str());

	//#
	//# use_auto_wb=0|1 - use auto white balance, averaging over entire image
	//#
	if (p.find("use_auto_wb") != p.end()) 
		RawProcessor.imgdata.params.use_auto_wb = atoi(p["use_auto_wb"].c_str());

	//#
	//# use_camera_wb=0|1 - use camera white balance, if available
	//#
	if (p.find("use_camera_wb") != p.end()) 
		RawProcessor.imgdata.params.use_camera_wb = atoi(p["use_camera_wb"].c_str());

	//#
	//# use_camera_matrix=0|1 - use camera color matrix
	//#
	if (p.find("use_camera_matrix") != p.end()) 
		RawProcessor.imgdata.params.use_camera_matrix = atoi(p["use_camera_matrix"].c_str());

	//#
	//# output_profile=filepath - use ICC profile from the file
	//#
	if (p.find("output_profile") != p.end()) 
		RawProcessor.imgdata.params.output_profile = (char *) p["output_profile"].c_str();

	//#
	//# camera_profile=filepath|embed - use ICC profile from the file
	//#
	if (p.find("camera_profile") != p.end()) 
		RawProcessor.imgdata.params.camera_profile = (char *) p["camera_profile"].c_str();

	//#
	//# bad_pixels=filepath - use bad pixel map
	//#
	if (p.find("bad_pixels") != p.end()) 
		RawProcessor.imgdata.params.bad_pixels = (char *) p["bad_pixels"].c_str();

	//#
	//# dark_frame=filepath - use bad pixel map
	//#
	if (p.find("dark_frame") != p.end()) 
		RawProcessor.imgdata.params.dark_frame = (char *) p["dark_frame"].c_str();

	//#
	//# output_bps=8|16 - bits per sample
	//#
	if (p.find("output_bps") != p.end()) 
		RawProcessor.imgdata.params.output_bps = atoi(p["output_bps"].c_str());

	//#
	//# output_tiff=0|1 - 0=output PPM, 1=output TIFF
	//#
	if (p.find("output_tiff") != p.end()) 
		RawProcessor.imgdata.params.output_tiff = atoi(p["output_tiff"].c_str());

	//#
	//# user_flip=0|1 - Flip image (0=none, 3=180, 5=90CCW, =90CW)
	//#
	if (p.find("user_flip") != p.end()) 
		RawProcessor.imgdata.params.user_flip = atoi(p["user_flip"].c_str());

	//#
	//# user_black=n - User black level
	//#
	if (p.find("user_black") != p.end()) 
		RawProcessor.imgdata.params.user_black = atoi(p["user_black"].c_str());

	//#
	//# user_cblack=rb,gb,bb,gb - user per-channel black levels, r,g,b,g
	//#
	if (p.find("user_cblack") != p.end()) {
		std::vector<std::string> c = split(p["user_cblack"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.user_cblack[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.user_cblack[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.user_cblack[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.user_cblack[3] = atoi(c[3].c_str());
		}
	}

	//
	// sony_arw2_hack=0|1 - Turn on/off division by four for Sony ARW
	//doesn't seem to be in libraw-17.2.0
	//if (p.find("sony_arw2_hack") != p.end()) 
	//	RawProcessor.imgdata.params.sony_arw2_hack = atoi(p["sony_arw2_hack"].c_str());

	//#
	//# user_sat=n - saturation
	//#
	if (p.find("user_sat") != p.end()) 
		RawProcessor.imgdata.params.user_sat = atoi(p["user_sat"].c_str());

	//#
	//# med_passes=n - number of median filter passes
	//#
	if (p.find("med_passes") != p.end()) 
		RawProcessor.imgdata.params.med_passes = atoi(p["med_passes"].c_str());

	//#
	//# auto_bright_thr=0.01 - portion of clipped pixel with autobright
	//#
	if (p.find("auto_bright_thr") != p.end()) 
		RawProcessor.imgdata.params.auto_bright_thr = atof(p["auto_bright_thr"].c_str());

	//#
	//# adjust_maximum_thr=0.01 - portion of clipped pixel with autobright
	//#
	if (p.find("adjust_maximum_thr") != p.end()) 
		RawProcessor.imgdata.params.adjust_maximum_thr = atof(p["adjust_maximum_thr"].c_str());

	//#
	//# use_fuji_rotate=0|1 - rotation for cameras with Fuji sensor
	//#
	if (p.find("use_fuji_rotate") != p.end()) 
		RawProcessor.imgdata.params.use_fuji_rotate = atoi(p["use_fuji_rotate"].c_str());

	//#
	//# green_matching=0|1 - fix green channel disbalance
	//#
	if (p.find("green_matching") != p.end()) 
		RawProcessor.imgdata.params.green_matching = atoi(p["green_matching"].c_str());

	//#
	//# dcb_iterations=n - number of DCB correction passes, -1 is no correction, default
	//#
	if (p.find("dcb_iterations") != p.end()) 
		RawProcessor.imgdata.params.dcb_iterations = atoi(p["dcb_iterations"].c_str());

	//#
	//# dcb_enhance_fl=n - DCB interpolation with enhanced interpolated colors
	//#
	if (p.find("dcb_enhance_fl") != p.end()) 
		RawProcessor.imgdata.params.dcb_enhance_fl = atoi(p["dcb_enhance_fl"].c_str());

	//#
	//# fbdd_noiserd=n - FBDD noise reduction, before demosaic
	//#
	if (p.find("fbdd_noiserd") != p.end()) 
		RawProcessor.imgdata.params.fbdd_noiserd = atoi(p["fbdd_noiserd"].c_str());

	//#
	//# eeci_refine=n - non-zero for ECCI refine for VCD interpolation
	//#
	if (p.find("eeci_refine") != p.end()) 
		RawProcessor.imgdata.params.eeci_refine = atoi(p["eeci_refine"].c_str());

	//#
	//# es_med_passes=n - number of edge-sensitive median filter passes after VCD+AHD demosaic
	//#
	if (p.find("es_med_passes") != p.end()) 
		RawProcessor.imgdata.params.es_med_passes = atoi(p["es_med_passes"].c_str());

	//#
	//# ca_correc=0|1 - chromatic aberration suppression
	//#
	if (p.find("ca_correc") != p.end()) 
		RawProcessor.imgdata.params.ca_correc = atoi(p["ca_correc"].c_str());

	//#
	//# cared=0.01 - chromatic aberration suppression red correction
	//#
	if (p.find("cared") != p.end()) 
		RawProcessor.imgdata.params.cared = atof(p["cared"].c_str());

	//#
	//# cablue=0.01 - chromatic aberration suppression blue correction
	//#
	if (p.find("cablue") != p.end()) 
		RawProcessor.imgdata.params.cablue = atof(p["cablue"].c_str());

	//#
	//# cfaline=0|1 - banding reduction
	//#
	if (p.find("cfaline") != p.end()) 
		RawProcessor.imgdata.params.cfaline = atoi(p["cfaline"].c_str());

	//#
	//# linenoise=0.01 - banding reduction amount
	//#
	if (p.find("linenoise") != p.end()) 
		RawProcessor.imgdata.params.linenoise = atof(p["linenoise"].c_str());

	//#
	//# cfa_clean=0|1 - Turns on impulse noise and Gaussian high frequency reduction
	//# lclean=0.005 to 0.05 - Amount of luminance reduction, 0.01 is a common value
	//# cclean=0.005 to 0.05 - Amount of color reduction, 0.01 is a common value
	//#
	if (p.find("cfa_clean") != p.end()) 
		RawProcessor.imgdata.params.cfa_clean = atoi(p["cfa_clean"].c_str());
	if (p.find("lclean") != p.end()) 
		RawProcessor.imgdata.params.lclean = atof(p["lclean"].c_str());
	if (p.find("cclean") != p.end()) 
		RawProcessor.imgdata.params.cclean = atof(p["cclean"].c_str());

	//#
	//# cfa_green=0|1 - Turns on reduction of maze artifacts produced by bad balance of green channels
	//# green_thresh=0.01 to 0.1 - Max difference between channels allowed for equalization 
	//#
	if (p.find("cfa_green") != p.end()) 
		RawProcessor.imgdata.params.cfa_green = atoi(p["cfa_green"].c_str());
	if (p.find("green_thresh") != p.end()) 
		RawProcessor.imgdata.params.green_thresh = atof(p["green_thresh"].c_str());

	//#
	//# exp_correc=0|1 - Turns on exposure correction before demosaic
	//# exp_shift=0.25 (2 stops darken) to 8.0 (3 stops lighten), default 1.0=no shift
	//# exp_preser=0.0 to 1.0 Preserve hightlights when lightening the image
	//#
	if (p.find("exp_correc") != p.end()) 
		RawProcessor.imgdata.params.exp_correc = atoi(p["exp_correc"].c_str());
	if (p.find("exp_shift") != p.end()) 
		RawProcessor.imgdata.params.exp_shift = atof(p["exp_shift"].c_str());
	if (p.find("exp_preser") != p.end()) 
		RawProcessor.imgdata.params.exp_preser = atof(p["exp_preser"].c_str());

	//#
	//# wf_debanding=0|1 - Turns on banding suppression
	//# wf_deband_threshold=tr,tg,tb,tg - Per-channel debanding thresholds
	//#
	if (p.find("wf_debanding") != p.end()) 
		RawProcessor.imgdata.params.wf_debanding = atoi(p["wf_debanding"].c_str());
	if (p.find("wf_deband_treshold") != p.end()) {
		std::vector<std::string> c = split(p["wf_deband_treshold"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.wf_deband_treshold[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.wf_deband_treshold[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.wf_deband_treshold[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.wf_deband_treshold[3] = atoi(c[3].c_str());
		}
	}

	//#
	//# no_auto_scale=0|1 - Disables pixel scaling, dcraw: -D.  Default=0
	//#
	if (p.find("no_auto_scale") != p.end()) 
		RawProcessor.imgdata.params.no_auto_scale = atoi(p["no_auto_scale"].c_str());

	//#
	//# no_interpolation=0|1 - Disables demosaic. dcraw: -d  Default=0
	//#
	if (p.find("no_interpolation") != p.end()) 
		RawProcessor.imgdata.params.no_interpolation = atoi(p["no_interpolation"].c_str());



	if (RawProcessor.open_file(filename) != LIBRAW_SUCCESS) return NULL;
	RawProcessor.unpack();
	
	*icclength = 0;

	info["ISOSpeedRatings"] = tostr(P2.iso_speed);  
	info["ExposureTime"] = tostr(P2.shutter);  
	info["FNumber"] = tostr(P2.aperture);  
	info["FocalLength"] = tostr(P2.focal_len);  
	info["ImageDescription"] = P2.desc;  
	info["Artist"] = P2.artist; 
	info["Make"] = P1.make;  
	info["Model"] = P1.model;  
	if (RawProcessor.imgdata.params.user_flip == 0) 
		info["Orientation"] = S.flip; //dcraw left the image alone, so set to what the raw file specified
	else
		info["Orientation"] = "0"; //dcraw flipped the image per the user's instruction (3, 5, 6) or the raw file specification (-1), so don't specify an orientation transform

	time_t rawtime = P2.timestamp;
	struct tm * timeinfo;
	char buffer [80];
	timeinfo = localtime (&rawtime);
	strftime (buffer,80,"%Y:%m:%d %H:%M:%S",timeinfo);
	info["DateTime"] = buffer;  
	
	if (rawdata) {
		*width = S.raw_width;
		*height = S.raw_height;
		*numcolors = 1;
		*numbits = 16;
		unsigned imgsize = (*width) * (*height) * (*numcolors) * ((*numbits)/8);
		img = new char[imgsize];
		memcpy(img, RawProcessor.imgdata.rawdata.raw_image, imgsize);

		RawProcessor.recycle();
		return img;
	}
	
	
	RawProcessor.dcraw_process();
	RawProcessor.get_mem_image_format(&w, &h, &c, &b);
	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;

	img = new char[w*h*c*(b/8)];
	
	libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image();
	memcpy(img, image->data, image->data_size);
	LibRaw::dcraw_clear_mem(image);

	//icc_m = NULL;


	if (C.profile) {
		*icc_m = new char[C.profile_length];
		memcpy(*icc_m, C.profile, C.profile_length);
		*icclength = C.profile_length;
	}
	else {  //because apparently libraw doesn't pass along the dcraw-generated profiles
		cmsHPROFILE profile;
		cmsUInt32Number size;
		float gamma = 1.0/RawProcessor.imgdata.params.gamm[0];
		if (RawProcessor.imgdata.params.output_color == 1) {
			profile = gImage::makeLCMSdcrawProfile("srgb", gamma);
			cmsSaveProfileToMem(profile, NULL, &size);
			*icclength = size;
			*icc_m = new char[size];
			cmsSaveProfileToMem(profile, *icc_m, &size);
		}
		if (RawProcessor.imgdata.params.output_color == 2) {
			profile = gImage::makeLCMSdcrawProfile("adobe", gamma);
			cmsSaveProfileToMem(profile, NULL, &size);
			*icclength = size;
			*icc_m = new char[size];
			cmsSaveProfileToMem(profile, *icc_m, &size);
		}
		if (RawProcessor.imgdata.params.output_color == 3) {
			profile = gImage::makeLCMSdcrawProfile("wide", gamma);
			cmsSaveProfileToMem(profile, NULL, &size);
			*icclength = size;
			*icc_m = new char[size];
			cmsSaveProfileToMem(profile, *icc_m, &size);
		}
		if (RawProcessor.imgdata.params.output_color == 4) {
			profile = gImage::makeLCMSdcrawProfile("prophoto", gamma);
			cmsSaveProfileToMem(profile, NULL, &size);
			*icclength = size;
			*icc_m = new char[size];
			cmsSaveProfileToMem(profile, *icc_m, &size);
		}
		if (RawProcessor.imgdata.params.output_color == 5) {
			profile = cmsCreateXYZProfile();
			cmsSaveProfileToMem(profile, NULL, &size);
			*icclength = size;
			*icc_m = new char[size];
			cmsSaveProfileToMem(profile, *icc_m, &size);
		}
	}


	RawProcessor.recycle();

	return img;

}


