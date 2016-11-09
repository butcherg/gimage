#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <vector>
#include <map>

#include "Curve.h"

struct pix {
	float r, g, b;
};

enum BPP {
	BPP_8,
	BPP_16
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
		gImage() { }
		gImage(std::string filename);
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo);
		gImage(unsigned width, unsigned height,  unsigned colors, std::map<std::string,std::string> imageinfo);

		~gImage();

		char *getImageData(BPP bits);
		std::vector<pix>& getImageData();
		unsigned getWidth();
		unsigned getHeight();
		unsigned getColors();
		std::map<std::string,std::string> getInfo();
		static int ThreadCount();

		void setInfo(std::string name, std::string value);

		void Stats();
		std::vector<long> Histogram();

		//Image operations
		gImage ConvolutionKernel(double kernel[3][3], int threadcount);
		gImage Sharpen(int strength, int threadcount);
		gImage Resize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount);
		gImage Rotate(double angle, int threadcount);
		gImage Crop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount);
		gImage Saturate(double saturate, int threadcount);
		gImage Tint(double red,double green,double blue, int threadcount);
		gImage Gray(double redpct, double greenpct, double bluepct, int threadcount);
		gImage ApplyCurve(std::vector<cp> ctpts, int threadcount);
		gImage ApplyLine(double low, double high, int threadcount);
		gImage NLMeans(double sigma, int local, int patch, int threadcount);

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


	private:
		std::vector<pix> image;
		unsigned w, h, c;
		std::map<std::string,std::string> imginfo;

		//Add:		
		//	icc

};



#endif
