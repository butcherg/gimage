#include "jpegimage.h"
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>

#include "LibRaw-0.17.2/libraw/libraw.h"
#include "jpeg-6b/jpeglib.h"

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

gImage::~gImage()
{
	free(img);
}


char * gImage::getImageData(unsigned bits)
{
	char *imagedata = (char*) malloc(w*h*c*(bits/8));

	if (bits == 16) {
		unsigned short * dst = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * src = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[0] = (unsigned short) (src->r*256.0);
				dst[1] = (unsigned short) (src->g*256.0);
				dst[2] = (unsigned short) (src->b*256.0);
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


gImage * gImage::loadRAW(const char * filename)
{
	int width, height, colors, bpp;
	LibRaw RawProcessor;

	RawProcessor.imgdata.params.shot_select = 0;
	RawProcessor.imgdata.params.use_camera_wb = 1;
	RawProcessor.imgdata.params.output_color = 1;	//sRGB, hardcoded for now
	RawProcessor.imgdata.params.user_qual = 3;	//AHD

	RawProcessor.imgdata.params.output_bps = 16;
	RawProcessor.imgdata.params.gamm[0] = 1/2.222;	//1;
	RawProcessor.imgdata.params.gamm[1] = 4.5;	//1;
	RawProcessor.imgdata.params.no_auto_bright = 0; //1;

	RawProcessor.open_file(filename);
	RawProcessor.unpack();
	RawProcessor.dcraw_process();
	RawProcessor.get_mem_image_format(&width, &height, &colors, &bpp);


	libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image();

	gImage * I = new gImage((char *) image->data, image->width,image->height,image->colors,image->bits);

	RawProcessor.recycle();

	return I;

}

gImage * gImage::loadJPEG(const char * filename)
{
	unsigned width, height, colors, bpp;
	char * image = 	_loadJPEG(filename, &width, &height, &colors);
	gImage * I = new gImage(image, width, height, colors, 8);
	free(image);
	return I;
}



gImage * gImage::saveJPEG(const char * filename)
{
	_writeJPEG(filename, getImageData(8),  w, h, c);
}




