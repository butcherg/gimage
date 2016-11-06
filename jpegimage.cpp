#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "jpeg-6b/jpeglib.h"

#ifdef __cplusplus
}
#endif


#include "jpegexif.h"

char * _loadJPEG(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			std::map<std::string,std::string> &info,
			std::string params="",
			char * icc_m=NULL, 
			unsigned  *icclength=0)
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

	jpeg_save_markers(&cinfo, JPEG_APP0+1, 0xFFFF);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_marker_struct * marker = cinfo.marker_list;
	std::map<std::string,std::string> imageinfo;

	while (marker != NULL) {
		parse_APP1marker(marker->data-2, marker->data_length, info);
		marker = marker->next;
	}

	jpeg_start_decompress(&cinfo);


	row_stride = cinfo.output_width * cinfo.output_components;
	img = (char *)malloc(cinfo.image_height * row_stride);
	dst = (JSAMPROW) img;

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}


	jpeg_finish_decompress(&cinfo);

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	*numcolors = cinfo.output_components;
	
	return img;

}





void _writeJPEG(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			std::map<std::string,std::string> info,
			std::string params="",
			char * icc_m=NULL, 
			unsigned  *icclength=0)
{

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	unsigned char *exif_data;
	unsigned int exif_data_len;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	unsigned char * marker;
	unsigned markerlength;

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
	cinfo.input_components = numcolors;	/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; /* colorspace of input image */

	jpeg_set_defaults(&cinfo);

	jpeg_start_compress(&cinfo, TRUE);

	marker =  construct_APP1marker(info, &markerlength);
	jpeg_write_marker(&cinfo, JPEG_APP0+1, marker+2, markerlength);
	free(marker);
	

	row_stride = cinfo.image_width * cinfo.input_components;
	//img = (char *)malloc(cinfo.image_height * row_stride);
	dst = (JSAMPROW) imagedata;

	while (cinfo.next_scanline < cinfo.image_height) {
		jpeg_write_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}

	jpeg_finish_compress(&cinfo);

}


