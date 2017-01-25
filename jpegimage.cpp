#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include <jpeglib.h>

#ifdef __cplusplus
}
#endif


#include "strutil.h"
#include "jpegexif.h"

bool _loadJPEGInfo(const char *filename, 
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

	if ((infile = fopen(filename, "rb")) == NULL) return false;

	jpeg_stdio_src(&cinfo, infile);

	jpeg_save_markers(&cinfo, JPEG_APP0+1, 0xFFFF);

	jpeg_read_header(&cinfo, TRUE);

	jpeg_marker_struct * marker = cinfo.marker_list;
	std::map<std::string,std::string> imageinfo;

	while (marker != NULL) {
		parse_APP1marker(marker->data-2, marker->data_length, info);
		marker = marker->next;
	}

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	*numcolors = cinfo.output_components;

	jpeg_destroy_decompress(&cinfo);

	return true;
}

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
	img = new char[cinfo.image_height * row_stride];
	dst = (JSAMPROW) img;

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}


	jpeg_finish_decompress(&cinfo);

	*width = cinfo.image_width;
	*height = cinfo.image_height;
	*numcolors = cinfo.output_components;

	jpeg_destroy_decompress(&cinfo);
	
	return img;

}


#define ICC_MARKER  (JPEG_APP0 + 2)	/* JPEG marker code for ICC */
#define ICC_OVERHEAD_LEN  14		/* size of non-profile data in APP2 */
#define MAX_BYTES_IN_MARKER  65533	/* maximum data len of a JPEG marker */
#define MAX_DATA_BYTES_IN_MARKER  (MAX_BYTES_IN_MARKER - ICC_OVERHEAD_LEN)


/*
 * This routine writes the given ICC profile data into a JPEG file.
 * It *must* be called AFTER calling jpeg_start_compress() and BEFORE
 * the first call to jpeg_write_scanlines().
 * (This ordering ensures that the APP2 marker(s) will appear after the
 * SOI and JFIF or Adobe markers, but before all else.)
 */

void write_icc_profile (j_compress_ptr cinfo,
		   const JOCTET *icc_data_ptr,
		   unsigned int icc_data_len)
{
  unsigned int num_markers;	/* total number of markers we'll write */
  int cur_marker = 1;		/* per spec, counting starts at 1 */
  unsigned int length;		/* number of bytes to write in this marker */

  /* Calculate the number of markers we'll need, rounding up of course */
  num_markers = icc_data_len / MAX_DATA_BYTES_IN_MARKER;
  if (num_markers * MAX_DATA_BYTES_IN_MARKER != icc_data_len)
    num_markers++;

  while (icc_data_len > 0) {
    /* length of profile to put in this marker */
    length = icc_data_len;
    if (length > MAX_DATA_BYTES_IN_MARKER)
      length = MAX_DATA_BYTES_IN_MARKER;
    icc_data_len -= length;

    /* Write the JPEG marker header (APP2 code and marker length) */
    jpeg_write_m_header(cinfo, ICC_MARKER,
			(unsigned int) (length + ICC_OVERHEAD_LEN));

    /* Write the marker identifying string "ICC_PROFILE" (null-terminated).
     * We code it in this less-than-transparent way so that the code works
     * even if the local character set is not ASCII.
     */
    jpeg_write_m_byte(cinfo, 0x49);
    jpeg_write_m_byte(cinfo, 0x43);
    jpeg_write_m_byte(cinfo, 0x43);
    jpeg_write_m_byte(cinfo, 0x5F);
    jpeg_write_m_byte(cinfo, 0x50);
    jpeg_write_m_byte(cinfo, 0x52);
    jpeg_write_m_byte(cinfo, 0x4F);
    jpeg_write_m_byte(cinfo, 0x46);
    jpeg_write_m_byte(cinfo, 0x49);
    jpeg_write_m_byte(cinfo, 0x4C);
    jpeg_write_m_byte(cinfo, 0x45);
    jpeg_write_m_byte(cinfo, 0x0);

    /* Add the sequencing info */
    jpeg_write_m_byte(cinfo, cur_marker);
    jpeg_write_m_byte(cinfo, (int) num_markers);

    /* Add the profile data */
    while (length--) {
      jpeg_write_m_byte(cinfo, *icc_data_ptr);
      icc_data_ptr++;
    }
    cur_marker++;
  }
}




void _writeJPEG(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			std::map<std::string,std::string> info,
			std::string params,
			char *iccprofile, 
			unsigned iccprofilelength)
{

	std::map<std::string,std::string> p = parseparams(params);

	info["UserComment"] = "This, is foo.";

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

	if (p.find("quality") != p.end()) 
		jpeg_set_quality(&cinfo, atoi(p["quality"].c_str()), TRUE);

	jpeg_start_compress(&cinfo, TRUE);

	marker =  construct_APP1marker(info, &markerlength);
	jpeg_write_marker(&cinfo, JPEG_APP0+1, marker+2, markerlength);
	delete marker;

	if (iccprofile) write_icc_profile (&cinfo, (const JOCTET *) iccprofile, iccprofilelength);

	row_stride = cinfo.image_width * cinfo.input_components;
	dst = (JSAMPROW) imagedata;

	while (cinfo.next_scanline < cinfo.image_height) {
		jpeg_write_scanlines(&cinfo, &dst, 1);
		dst += row_stride;
	}

	jpeg_finish_compress(&cinfo);

	jpeg_destroy_compress(&cinfo);

}



