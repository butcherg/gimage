#ifndef _jpegimage_h
#define _jpegimage_h

#ifdef __cplusplus
extern "C" {
#endif

char * _loadJPEG(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors);
void _writeJPEG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors);

#ifdef __cplusplus
}
#endif

#endif

