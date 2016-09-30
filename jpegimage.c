#include <stdio.h>
#include <stdlib.h>
#include "jpeg-6b/jpeglib.h"

char * loadJPEG(const char *filename, char *imagedata, unsigned *width, unsigned *height, unsigned *components)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	char *img;
	JSAMPROW dst;
	unsigned row_stride;
	FILE * infile;

	if ((infile = fopen(filename, "rb")) == NULL) {
	    fprintf(stderr, "can't open %s\n", filename);
	    exit(1);
	}
	jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);


	jpeg_start_decompress(&cinfo);

	//printf("Image name: %s, width: %d, height %d\n", filename, cinfo.output_width, cinfo.output_height);
	//printf("Image name: %s, width: %d, height %d, precision: %d\n", filename, cinfo.image_width, cinfo.image_height, cinfo.num_components);

	row_stride = cinfo.output_width * cinfo.output_components;
	img = (char *)malloc(cinfo.image_height * row_stride);
	dst = img;

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}


	jpeg_finish_decompress(&cinfo);

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	*components = cinfo.output_components;

	imagedata = img;
	
	return img;

}


void writeJPEG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned components)
{

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

	cinfo.image_width = width; 	/* image width and height, in pixels */
	cinfo.image_height = height;
	cinfo.input_components = components;	/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

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

}


