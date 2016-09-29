#include <stdio.h>
#include <stdlib.h>
#include "jpeg-6b/jpeglib.h"

void loadJPEG(const char *filename, void *imagedata, unsigned *width, unsigned *height)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	FILE * infile;

	if ((infile = fopen(filename, "rb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", filename);
	    exit(1);
	}
	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_start_decompress(&cinfo);



	jpeg_finish_decompress(&cinfo);


}


void writeJPEG(const char *filename, void *imagedata, unsigned *width, unsigned *height)
{

}


