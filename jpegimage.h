#ifndef _jpegimage_h
#define _jpegimage_h

char * _loadJPEG(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors);
void _writeJPEG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors);


#endif

