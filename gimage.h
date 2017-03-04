#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <vector>
#include <map>
#include <lcms2.h>
#include "curve.h"
#include "half.hpp"

#define VERSION "0.1"

#ifndef PIXTYPE
#define PIXTYPE float
#endif

using half_float::half;

struct pix {
	PIXTYPE r, g, b;
};

enum BPP {
	BPP_FP,
	BPP_8,
	BPP_16
};

enum GIMAGE_CHANNEL {
	CHANNEL_RED,
	CHANNEL_GREEN,
	CHANNEL_BLUE
};

enum GIMAGE_FILETYPE {
	FILETYPE_RAW,
	FILETYPE_JPEG,
	FILETYPE_TIFF,
	FILETYPE_UNKNOWN
};

enum RESIZE_FILTER {
	FILTER_BOX,
	FILTER_BILINEAR,
	FILTER_BSPLINE,
	FILTER_BICUBIC,
	FILTER_CATMULLROM,
	FILTER_LANCZOS3
};


class gImage 
{
	public:
		gImage();
		gImage(const gImage &o);
		gImage(std::string filename);
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo, char * icc_profile=NULL, unsigned icc_profile_length=0);
		gImage(unsigned width, unsigned height,  unsigned colors, std::map<std::string,std::string> imageinfo);

		~gImage();

		//Getters
		pix getPixel(unsigned x,  unsigned y);
		std::vector<float> getPixelArray(unsigned x,  unsigned y);
		char *getImageData(BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		char *getTransformedImageData(BPP bits, cmsHPROFILE profile, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		std::vector<pix>& getImageData();
		pix* getImageDataRaw();
		unsigned getWidth();
		unsigned getHeight();
		unsigned getColors();
		BPP getBits();
		std::string getBitsStr();
		std::map<std::string,std::string> getInfo();
		char * getProfile();
		unsigned getProfileLength();
		std::string Stats();
		std::vector<long> Histogram();
		std::map<GIMAGE_CHANNEL,std::vector<unsigned> > Histogram(unsigned channels, unsigned scale);
		std::vector<unsigned> Histogram(unsigned channel, unsigned scale, unsigned &hmax);

		//Setters
		void setInfo(std::string name, std::string value);
		void setProfile(char * prof, unsigned proflength);

		//Static methods
		static std::string getRGBCharacteristics();
		static std::map<std::string,std::string> getInfo(const char * filename);
		static GIMAGE_FILETYPE getFileType(const char * filename);
		static GIMAGE_FILETYPE getFileNameType(const char * filename);
		static int ThreadCount();
		static std::string Version();

		//Image operations.  
		//threadcount=0 uses all available CPUs, n uses precisely n CPUs, and -n uses available-n CPUs
		void ApplyConvolutionKernel(double kernel[3][3], int threadcount=0);
		void ApplySharpen(int strength, int threadcount=0);
		void ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount=0);
		void ApplyRotate(double angle, bool crop, int threadcount=0);
		void ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount=0);
		void ApplySaturate(double saturate, int threadcount=0);
		void ApplyTint(double red,double green,double blue, int threadcount=0);
		void ApplyGray(double redpct, double greenpct, double bluepct, int threadcount=0);
		void ApplyToneCurve(std::vector<cp> ctpts, int threadcount=0);
		void ApplyToneLine(double low, double high, int threadcount=0);
		void ApplyNLMeans(double sigma, int local, int patch, int threadcount=0);
		

		//Image loaders.  Return a new gImage
		static gImage loadRAW(const char * filename, std::string params);
		static gImage loadJPEG(const char * filename, std::string params);
		static gImage loadTIFF(const char * filename, std::string params);
		static gImage loadImageFile(const char * filename, std::string params);

		//Image savers. 
		bool saveImageFile(const char * filename, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		void saveJPEG(const char * filename, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		void saveTIFF(const char * filename, BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//ICC (LittleCMS) profiles.
		static cmsHPROFILE makeLCMSProfile(const std::string name, float gamma);
		static void makeICCProfile(cmsHPROFILE hProfile, char *& profile, cmsUInt32Number  &profilesize);

	protected:

		//void ImageBounds(unsigned *x1, unsigned *x2, unsigned *y1, unsigned *y2, bool cropbounds=false);
		//void ApplyXShear(double rangle, int threadcount);
		//void ApplyYShear(double rangle, int threadcount);


	private:
		std::vector<pix> image;
		unsigned w, h, c;
		BPP b;
		std::map<std::string,std::string> imginfo;

		char *profile;
		unsigned profile_length;
		

};



#endif
