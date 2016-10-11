#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiffio.h"

#include <string>
#include <map>

#include "strutil.h"

/*
char * _loadTIFF(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits)
{
	char *img, *buf;
	FILE * infile;
	uint16 w, h, c, b;

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

		if (c != 3) {printf("c=%d\n",c); return NULL;}

		img = (char*) malloc(w*h*c*(b/8));
		unsigned short * dst = (unsigned short *) img;

		buf = (char *) _TIFFmalloc(TIFFScanlineSize(tif));
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
	uint16 w, h, c, b;

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
*/


char * _loadTIFF(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info)
{
	char *img, *buf;
	FILE * infile;
	uint16 w, h, c, b;

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

		char *infobuf;
		if (TIFFGetField(tif, TIFFTAG_ARTIST, &infobuf)) info["artist"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_MAKE, &infobuf))  info["make"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_MODEL, &infobuf))  info["model"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_SOFTWARE, &infobuf))  info["software"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_COPYRIGHT, &infobuf))  info["copyright"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_LENSINFO, &infobuf))  info["lensinfo"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_DATETIME, &infobuf)) {
			struct tm timestruct;
			timestruct.tm_year  = atoi(strtok(infobuf, ":"))-1900;
			timestruct.tm_mon   = atoi(strtok(NULL, ":"))-1;
			timestruct.tm_mday  = atoi(strtok(NULL, " "));
			timestruct.tm_hour  = atoi(strtok(NULL, ":"));
			timestruct.tm_min   = atoi(strtok(NULL, ":"));
			timestruct.tm_sec   = atoi(strtok(NULL, " "));
			info["shottime"] = tostr(mktime(&timestruct));
		}
		



		if (b != 16) return NULL;
		if (c != 3) return NULL;

		img = (char*) malloc(w*h*c*(b/8));
		unsigned short * dst = (unsigned short *) img;

		buf = (char *) _TIFFmalloc(TIFFScanlineSize(tif));
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

void _writeTIFF(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info)
{
	char *img;
	unsigned char *buf;
	uint16 w, h, c, b;

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

		if (info.find("artist") != info.end())  TIFFSetField(tif, TIFFTAG_ARTIST, info["artist"].c_str());
		if (info.find("make") != info.end())  TIFFSetField(tif, TIFFTAG_MAKE, info["make"].c_str());
		if (info.find("model") != info.end())  TIFFSetField(tif, TIFFTAG_MODEL, info["model"].c_str());
		if (info.find("software") != info.end())  TIFFSetField(tif, TIFFTAG_SOFTWARE, info["software"].c_str());
		if (info.find("copyright") != info.end())  TIFFSetField(tif, TIFFTAG_COPYRIGHT, info["copyright"].c_str());
		if (info.find("lensinfo") != info.end())  TIFFSetField(tif, TIFFTAG_LENSINFO, info["lensinfo"].c_str());

		if (info.find("shottime") != info.end()) {
			char timestr[256];
			struct tm *tmp;
			time_t t;
			t = (time_t)  atoi(info["shottime"].c_str());
			tmp = gmtime(&t);
			if (strftime(timestr, 255, "%Y:%m:%d %T %Z", tmp))
				TIFFSetField(tif, TIFFTAG_DATETIME, timestr);
		}
		if (info.find("imagedescription") != info.end())  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, info["imagedescription"].c_str());
		


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

