#ifndef _gimage_h
#define _gimage_h

struct pix {
	double r, g, b;
};


class gImage 
{
	public:
		gImage() { }
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits);
		~gImage();
		char *getImageData(unsigned bits);
		unsigned getWidth();
		unsigned getHeight();
		unsigned getColors();


		//Image loaders.  Return a new gImage
		static gImage * loadRAW(const char * filename);
		static gImage * loadJPEG(const char * filename);
		static gImage * loadTIFF(const char * filename);

		//Image savers. 
		void saveJPEG(const char * filename);
		void saveTIFF(const char * filename, unsigned bits);


	private:
		pix * img;
		unsigned w, h, c;
		//Add:
		//	exif?
		//	icc

};



#endif
