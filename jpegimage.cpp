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

char * _loadJPEG(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, std::map<std::string,std::string> &info)
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
		ed = exif_data_new_from_data( marker->data, marker->data_length);
		char value[1024];
		for (unsigned i = 0; i < EXIF_IFD_COUNT; i++) {
			if (ed->ifd[i] && ed->ifd[i]->count) {
 				ExifContent *content = ed->ifd[i];
				for (unsigned j = 0; j < content->count; j++) {
					ExifEntry *e = content->entries[j];
					const ExifByteOrder o = exif_data_get_byte_order (e->parent->parent);
					//exif_entry_dump(e,0);
 					sprintf(name, "%s", exif_tag_get_name_in_ifd (e->tag, exif_entry_get_ifd(e)));
					switch (e->format) {
						case EXIF_FORMAT_ASCII:
							strncpy (val, (char *) e->data, 1024);
							val[e->size] = 0;
							//printf("%s\n", val);
							break;

						case  EXIF_FORMAT_BYTE:
						case  EXIF_FORMAT_SBYTE:
							printf("%a\n", e->data[0]);
							break;
						case  EXIF_FORMAT_SHORT:
							v_short = exif_get_short (e->data, o);
							snprintf (val, 1024, "%u", v_short);
							//printf("%s\n",val);
							break;
						case  EXIF_FORMAT_LONG:
							v_long = exif_get_long (e->data, o);
							snprintf (val, 1024, "%lu", (unsigned long) v_long);
							//printf("%s\n",val);
							break;
						case  EXIF_FORMAT_RATIONAL:
							v_rat = exif_get_rational (e->data, o);
							if (v_rat.denominator == 0) sprintf(val, "%lu", v_rat.numerator); else
							sprintf(val, "%g",(double) v_rat.numerator/ (double) v_rat.denominator);
							//printf("%s\n",val);
							break;
						case  EXIF_FORMAT_SSHORT:
							v_sshort = exif_get_sshort (e->data, o);
							snprintf (val, 1024, "%i", v_sshort);
							//printf("%s\n",val);
							break;
						case  EXIF_FORMAT_SLONG:
							v_slong = exif_get_slong (e->data, o);
							snprintf (val, 1024, "%li", (long) v_slong);
							//printf("%s\n",val);
							break;
						case  EXIF_FORMAT_SRATIONAL:
							v_srat = exif_get_srational (e->data, o);
							if (v_srat.denominator == 0) sprintf(val, "%lu", v_srat.numerator); else
							sprintf(val, "%g",(double) v_srat.numerator/ (double) v_srat.denominator);
							//printf("%s\n",val);
							break;
						case  EXIF_FORMAT_UNDEFINED:
						case  EXIF_FORMAT_FLOAT:
						case  EXIF_FORMAT_DOUBLE:
							printf("(undefined)\n");

					}
					info[name] = val;
				}
			}
		}
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

ExifEntry *init_tag(ExifData *exif, ExifIfd ifd, ExifTag tag)
{
	ExifEntry *entry;
	/* Return an existing tag if one exists */
	if (!((entry = exif_content_get_entry (exif->ifd[ifd], tag)))) {
	    /* Allocate a new entry */
	    entry = exif_entry_new ();
	    //assert(entry != NULL); /* catch an out of memory condition */
	    entry->tag = tag; /* tag must be set before calling
				 exif_content_add_entry */

	    /* Attach the ExifEntry to an IFD */
	    exif_content_add_entry (exif->ifd[ifd], entry);

	    /* Allocate memory for the entry and fill with default data */
	    exif_entry_initialize (entry, tag);

	    /* Ownership of the ExifEntry has now been passed to the IFD.
	     * One must be very careful in accessing a structure after
	     * unref'ing it; in this case, we know "entry" won't be freed
	     * because the reference count was bumped when it was added to
	     * the IFD.
	     */
	    exif_entry_unref(entry);
	}
	return entry;
}

void AddEntry(ExifData *exif, ExifIfd ifd, std::string name, std::string value) 
{
	ExifEntry *entry;
	entry = exif_entry_new ();
	entry->tag = exif_tag_from_name(name.c_str());
	exif_content_add_entry (exif->ifd[ifd], entry);
	exif_entry_initialize (entry, entry->tag);

	switch (entry->format) {
		case EXIF_FORMAT_BYTE:
			break;
		case EXIF_FORMAT_ASCII:
			break;
		case EXIF_FORMAT_SHORT:
			break;
		case EXIF_FORMAT_LONG:
			break;
		case EXIF_FORMAT_RATIONAL:
			break;
		case EXIF_FORMAT_SBYTE:
			break;
		case EXIF_FORMAT_UNDEFINED:
			break;
		case EXIF_FORMAT_SSHORT:
			break;
		case EXIF_FORMAT_SLONG:
			break;
		case EXIF_FORMAT_SRATIONAL:
			break;
		case EXIF_FORMAT_FLOAT:
			break;
		case EXIF_FORMAT_DOUBLE:
			break;
	}

	exif_entry_unref(entry);
}


void _writeJPEG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned components, std::map<std::string,std::string> info)
{

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	unsigned char *exif_data;
	unsigned int exif_data_len;
	ExifEntry *entry;
	ExifData *exif = exif_data_new();

	exif_data_set_option(exif, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
	exif_data_set_data_type(exif, EXIF_DATA_TYPE_COMPRESSED);
	exif_data_set_byte_order(exif, EXIF_BYTE_ORDER_INTEL);

	/* Create the mandatory EXIF fields with default data */
	exif_data_fix(exif);

	/* All these tags are created with default values by exif_data_fix() */
	/* Change the data to the correct values for this image. */
	entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
	exif_set_long(entry->data, EXIF_BYTE_ORDER_INTEL, width);

	entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
	exif_set_long(entry->data, EXIF_BYTE_ORDER_INTEL, height);

	entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE);
	exif_set_short(entry->data, EXIF_BYTE_ORDER_INTEL, 1);

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
	cinfo.input_components = components;	/* # of color components per pixel */
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


