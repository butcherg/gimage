#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiffio.h"

char * _loadTIFF(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits)
{
	char *img, *buf;
	FILE * infile;
	uint32 w, h, c, b;

	TIFF* tif = TIFFOpen(filename, "r");
	if (tif) {

		size_t npixels;
		uint32* raster;

		uint32 imagelength, imagewidth;
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
		}
		TIFFClose(tif);
	}

	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;
	if (buf) _TIFFfree(buf);
	return img;
}


void _writeTIFF(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits)
{
	char *img;
	unsigned char *buf;
	uint32 w, h, c, b;

	TIFF* tif = TIFFOpen(filename, "w");
	if (tif) {

		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);  
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);    
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, numcolors);   
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, numbits);   
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);   

		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		// We set the strip size of the file to be size of one row of pixels
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, width*numcolors));

		unsigned scanlinesize = TIFFScanlineSize(tif);
		buf = (unsigned char *) _TIFFmalloc(scanlinesize);
		img = imagedata;

		for (unsigned row = 0; row < height; row++)
		{
			memcpy(buf, img, scanlinesize);
			if (TIFFWriteScanline(tif, buf, row, 0) < 0) {
				printf("TIFFWriteScanline got an error...\n");
				TIFFError(NULL,NULL);
				break;
			}
			img+=scanlinesize;
		}
	}

	(void) TIFFClose(tif);
	if (buf) _TIFFfree(buf);
}


