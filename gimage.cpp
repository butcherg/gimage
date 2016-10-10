
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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
	if (bits == 0) img = (pix *) imagedata; //supplied by caller
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
	if (bits == 0) img = (pix *) imagedata; // imagedata is already pix *

	imginfo = imageinfo;
	

}


gImage::~gImage()
{
	free(img);
}


//Getters:


gImage * gImage::Copy()
{
	pix * i = (pix *) malloc(w*h*sizeof(pix));
	memcpy(i, img, w*h*sizeof(pix));
	std::map<std::string,std::string> info = imginfo;
	return new gImage((char *)i, w, h, c, 0, info);
}


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

				//dst[0] = (char) std::min(std::max(int(src[x].r),0),255); 
				//dst[1] = (char) std::min(std::max(int(src[x].g),0),255);
				//dst[2] = (char) std::min(std::max(int(src[x].b),0),255); 
				dst += 3;
			}
		}
	}
	return imagedata;
}

pix * gImage::getImageData()
{
	return img;
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


//Image Operations:

gImage * gImage::ConvolutionKernel(double kernel[3][3], int threadcount)
{
	gImage *S = Copy();
	pix * cpy = S->getImageData();

	//#pragma omp parallel for num_threads(threadcount)
	for(unsigned y = 1; y < h-1; y++) {
		for(unsigned x = 1; x < w-1; x++) {
			pix * dst = (pix *) (cpy + w*y + x);
			double R=0.0; double G=0.0; double B=0.0;
			for (unsigned kx=0; kx<3; kx++) {
				for (int ky=0; ky<3; ky++) {
					pix * src = (pix *) (img + w*(y-1+ky) + (x-1+kx));
					R += src->r * kernel[kx][ky];
					G += src->g * kernel[kx][ky];
					B += src->b * kernel[kx][ky];
				}
				dst->r = R;
				dst->g = G;
				dst->b = B;

				//dst->r = floor(std::min(std::max(R, 0.0), 255.0));
				//dst->g = floor(std::min(std::max(G, 0.0), 255.0));
				//dst->b = floor(std::min(std::max(B, 0.0), 255.0));
			}


		}
	} 

	return S;
}



gImage * gImage::Sharpen(int strength, int threadcount)
{
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	double x = -((strength)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = strength+1;

	return ConvolutionKernel(kernel, threadcount);

}


//Loaders:

gImage * gImage::loadRAW(const char * filename)
{
	unsigned width, height, colors, bpp, icclength;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadRAW_m(filename, &width, &height, &colors, &bpp, imgdata, iccprofile, &icclength);
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
	std::map<std::string,std::string> imgdata;

	char * image = _loadTIFF(filename, &width, &height, &colors, &bpp, imgdata);
	gImage * I = new gImage(image, width, height, colors, bpp, imgdata);

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
	_writeTIFF(filename, getImageData(bits),  w, h, c, bits, imginfo);
}




