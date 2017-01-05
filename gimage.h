#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <vector>
#include <map>
#include "curve.h"

#define VERSION "0.1"

struct pix {
	float r, g, b;
};

enum BPP {
	BPP_8,
	BPP_16
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
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo, void * icc_profile=NULL, unsigned icc_profile_length=0);
		gImage(unsigned width, unsigned height,  unsigned colors, std::map<std::string,std::string> imageinfo);

		~gImage();

		//Getters
		pix getPixel(unsigned x,  unsigned y);
		char* getImageData(BPP bits);
		pix* getImageData();
		unsigned getWidth();
		unsigned getHeight();
		unsigned getColors();
		std::map<std::string,std::string> getInfo();
		char * getProfile();
		unsigned getProfileLength();
		std::string Stats();
		std::vector<long> Histogram();

		//Setters
		void setInfo(std::string name, std::string value);

		//Static methods
		static std::map<std::string,std::string> getInfo(const char * filename);
		static GIMAGE_FILETYPE getFileType(const char * filename);
		static int ThreadCount();
		static std::string Version();

/*
		//Image operations
		gImage ConvolutionKernel(double kernel[3][3], int threadcount);
		gImage Sharpen(int strength, int threadcount);
		gImage Resize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount);
		gImage Rotate(double angle, int threadcount);
		gImage Crop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount);
		gImage Saturate(double saturate, int threadcount);
		gImage Tint(double red,double green,double blue, int threadcount);
		gImage Gray(double redpct, double greenpct, double bluepct, int threadcount);
		gImage ToneCurve(std::vector<cp> ctpts, int threadcount);
		gImage ToneLine(double low, double high, int threadcount);
		gImage NLMeans(double sigma, int local, int patch, int threadcount);
*/

		//In-place versions of image operations:
		void ApplyConvolutionKernel(double kernel[3][3], int threadcount);
		void ApplySharpen(int strength, int threadcount);
		void ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount);
		void ApplyRotate(double angle, int threadcount);
		void ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount);
		void ApplySaturate(double saturate, int threadcount);
		void ApplyTint(double red,double green,double blue, int threadcount);
		void ApplyGray(double redpct, double greenpct, double bluepct, int threadcount);
		void ApplyToneCurve(std::vector<cp> ctpts, int threadcount);
		void ApplyToneLine(double low, double high, int threadcount);
		void ApplyNLMeans(double sigma, int local, int patch, int threadcount);
		

		//Image loaders.  Return a new gImage
		static gImage loadRAW(const char * filename, std::string params);
		static gImage loadJPEG(const char * filename, std::string params);
		static gImage loadTIFF(const char * filename, std::string params);
		static gImage loadImageFile(const char * filename, std::string params);

		//Image savers. 
		bool saveImageFile(const char * filename, std::string params="");
		void saveJPEG(const char * filename, std::string params="");
		void saveTIFF(const char * filename, BPP bits);

	protected:
		gImage XShear(double rangle, int threadcount);
		gImage YShear(double rangle, int threadcount);
		void ImageBounds(unsigned *x1, unsigned *x2, unsigned *y1, unsigned *y2);

		void ApplyXShear(double rangle, int threadcount);
		void ApplyYShear(double rangle, int threadcount);


	private:
		//std::vector<pix> image;
		pix* image;
		unsigned w, h, c;
		std::map<std::string,std::string> imginfo;

		char *profile;
		unsigned profile_length;
		

		//Add:		
		//	icc

};



#endif
