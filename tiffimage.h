#ifndef _tiffimage_h
#define _tiffimage_h

#ifdef __cplusplus
extern "C" {
#endif

char * _loadTIFF(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits);
void _writeTIFF(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits);

#ifdef __cplusplus
}
#endif

#endif

