#include <stdio.h>
#include <stdlib.h>
#include "tiff-4.0.6/libtiff/tiffio.h"

char * _loadTIFF(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits)
{
	char *img;
	FILE * infile;
	uint32 w, h, c, b;

	TIFF* tif = TIFFOpen(filename, "r");
	if (tif) {

		size_t npixels;
		uint32* raster;

		uint32 imagelength, imagewidth;
		/*tdata_t buf;*/
		char *buf;
		uint16 config, nsamples;
        
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &c);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &b);
		TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

		if (b != 16) return NULL;
		if (c != 3) return NULL;

		img = (char*) malloc(w*h*c*(b/8));
		unsigned short * dst = (unsigned short *) img;

		buf = _TIFFmalloc(TIFFScanlineSize(tif));
		unsigned short * src = (unsigned short *) buf;

		if (config == PLANARCONFIG_CONTIG) {
			for (unsigned y = 0; y < h; y++){
				TIFFReadScanline(tif, buf, y, 0);
				src = (unsigned short *) buf;
				for(unsigned x=0; x < w; x++) {
					if (c == 1) {
						uint16 gray =	(uint16) buf[x*c+0];
					}
					else if(c == 3 ){
						dst[0] = (unsigned short) src[0];
						dst[1] = (unsigned short) src[1];
						dst[2] = (unsigned short) src[2];
						dst+=3;
						src+=3;
					}
				}
			}
			TIFFClose(tif);
		}
	}

	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;
	return img;
}


void _writeTIFF(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors)
{
/*
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	FILE * outfile;
	unsigned row_stride;
	JSAMPROW dst;

	if ((outfile = fopen(filename, "wb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", filename);
	    exit(1);
	}
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = width; 	
	cinfo.image_height = height;
	cinfo.input_components = components;	
	cinfo.in_color_space = JCS_RGB; 

	jpeg_set_defaults(&cinfo);

	jpeg_start_compress(&cinfo, TRUE);

	row_stride = cinfo.image_width * cinfo.input_components;
	//img = (char *)malloc(cinfo.image_height * row_stride);
	dst = imagedata;

	while (cinfo.next_scanline < cinfo.image_height) {
		jpeg_write_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}

	jpeg_finish_compress(&cinfo);
*/
}


