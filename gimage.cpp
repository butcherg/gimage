
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>

#include "rawimage.h"
#include "jpegimage.h"
#include "tiffimage.h"


//Constructor/Destructor:

gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits)
{
	img = (pix *) malloc(width*height*sizeof(pix));
	w=width;
	h=height;
	c=colors;

	if (bits == 16) {
		unsigned short * src = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[x].r = src[0]/256;
				dst[x].g = src[1]/256;
				dst[x].b = src[2]/256;
				src += 3;
			}
		}
	}					

	if (bits == 8) {
		char * src = (char *) imagedata;
		for (unsigned y=0; y<height; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<width; x++) {
				dst[x].r = src[0];
				dst[x].g = src[1];
				dst[x].b = src[2];
				src += 3;
			}
		}
	}
}

gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits, std::map<std::string,std::string> imageinfo)
{
	img = (pix *) malloc(width*height*sizeof(pix));
	w=width;
	h=height;
	c=colors;

	if (bits == 16) {
		unsigned short * src = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[x].r = src[0]/256;
				dst[x].g = src[1]/256;
				dst[x].b = src[2]/256;
				src += 3;
			}
		}
	}					

	if (bits == 8) {
		char * src = (char *) imagedata;
		for (unsigned y=0; y<height; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<width; x++) {
				dst[x].r = src[0];
				dst[x].g = src[1];
				dst[x].b = src[2];
				src += 3;
			}
		}
	}
	imginfo = imageinfo;

}

gImage::~gImage()
{
	free(img);
}


//Getters:

char * gImage::getImageData(unsigned bits)
{
	char *imagedata = (char*) malloc(w*h*c*(bits/8));

	if (bits == 16) {
		unsigned short * dst = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * src = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[0] = (unsigned short) (src[x].r*256.0);
				dst[1] = (unsigned short) (src[x].g*256.0);
				dst[2] = (unsigned short) (src[x].b*256.0);

				//dst[0] = (unsigned short) (src->r*256.0);
				//dst[1] = (unsigned short) (src->g*256.0);
				//dst[2] = (unsigned short) (src->b*256.0);
				dst += 3;
			}
		}
	}

	if (bits == 8) {
		char * dst = (char *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * src = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[0] = (char) src[x].r; 
				dst[1] = (char) src[x].g; 
				dst[2] = (char) src[x].b; 
				dst += 3;
			}
		}
	}
	return imagedata;
}

unsigned gImage::getWidth()
{
	return w;
}

unsigned gImage::getHeight()
{
	return h;
}

unsigned gImage::getColors()
{
	return c;
}

std::map<std::string,std::string> gImage::getInfo()
{
	return imginfo;
}



//Loaders:

gImage * gImage::loadRAW(const char * filename)
{
	unsigned width, height, colors, bpp, icclength;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadRAW1_m(filename, &width, &height, &colors, &bpp, imgdata, iccprofile, &icclength);
	gImage * I = new gImage(image, width, height, colors, bpp, imgdata);
	free(image);
	return I;

}

gImage * gImage::loadJPEG(const char * filename)
{
	unsigned width, height, colors, bpp;
	char * image = _loadJPEG(filename, &width, &height, &colors);
	gImage * I = new gImage(image, width, height, colors, 8);
	free(image);
	return I;
}


gImage * gImage::loadTIFF(const char * filename)
{
	unsigned width, height, colors, bpp;
	char * image = _loadTIFF(filename, &width, &height, &colors, &bpp);
	gImage * I = new gImage(image, width, height, colors, bpp);
	free(image);
	return I;
}


//Savers:

void gImage::saveJPEG(const char * filename)
{
	_writeJPEG(filename, getImageData(8),  w, h, c);
}

void gImage::saveTIFF(const char * filename, unsigned bits)
{
	_writeTIFF(filename, getImageData(bits),  w, h, c, bits);
}




