#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		}
		TIFFClose(tif);
	}

	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;
	return img;
}


void _writeTIFF(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits)
{
	char *img;
	unsigned char *buf;
	uint32 w, h, c, b;

	TIFF* tif = TIFFOpen(filename, "w");
	if (tif) {

		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);  // set the width of the image
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);    // set the height of the image
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, numcolors);   // set number of channels per pixel
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, numbits);    // set the size of the channels
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);    // set the origin of the image.
		//   Some other essential fields to set that you do not have to understand for now.
		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

		size_t linebytes = numcolors * (numbits/8) * width;     // length in memory of one row of pixel in the image.
		buf = NULL;        // buffer used to store the row of pixel information for writing to file

printf("scan line size: %lu\n",TIFFScanlineSize(tif));

		unsigned scanlinesize = TIFFScanlineSize(tif);

		//    Allocating memory to store the pixels of current row
		//if (TIFFScanlineSize(tif) == linebytes)
		//	buf =(unsigned char *)_TIFFmalloc(linebytes);
		//else
		//	buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));
		buf = (unsigned char *) _TIFFmalloc(scanlinesize);

		// We set the strip size of the file to be size of one row of pixels
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, width*numcolors));

		//Now writing image to the file one strip at a time
		img = imagedata;

		unsigned row;
printf("about to write %d scanlines.\n",height);
		for ( row = 0; row < height; row++)
		{
			//memcpy(buf, &imagedata[(h-row-1)*linebytes], linebytes);    // check the index here, and figure out why not using h*linebytes
			memcpy(buf, &img, scanlinesize-1);
			if (TIFFWriteScanline(tif, buf, row, 0) < 0) {
				printf("TIFFWriteScanline got an error...\n");
				TIFFError(NULL,NULL);
				break;
			}
			img+=scanlinesize;
		}
printf("%d rows written.\n",row);

	}

	(void) TIFFClose(tif);
	if (buf) _TIFFfree(buf);
}


