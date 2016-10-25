#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <map>

struct pix {
	double r, g, b;
};


class gImage 
{
	public:
		gImage() { }
		gImage(std::string filename);
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits);
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits, std::map<std::string,std::string> imageinfo);
		//gImage::gImage(pix * imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits, std::map<std::string,std::string> imageinfo);
		~gImage();

		gImage * Copy();
		char *getImageData(unsigned bits);
		pix *getImageData();
		unsigned getWidth();
		unsigned getHeight();
		unsigned getColors();
		std::map<std::string,std::string> getInfo();

		void Stats();

		//Image operations
		gImage * ConvolutionKernel(double kernel[3][3], int threadcount);
		gImage * Sharpen(int strength, int threadcount);


		//Image loaders.  Return a new gImage
		static gImage * loadRAW(const char * filename, std::string params);
		static gImage * loadJPEG(const char * filename, std::string params);
		static gImage * loadTIFF(const char * filename, std::string params);
		static gImage * loadImageFile(const char * filename, std::string params);
		//Image savers. 
		void saveJPEG(const char * filename);
		void saveTIFF(const char * filename, unsigned bits);
		bool saveImageFile(const char * filename);


	private:
		pix * img;
		unsigned w, h, c;
		std::map<std::string,std::string> imginfo;

		//Add:		
		//	icc

};



#endif
