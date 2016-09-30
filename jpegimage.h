#ifndef _jpegimage_h
#define _jpegimage_h

#ifdef __cplusplus
extern "C" {
#endif

char * loadJPEG(const char *filename, char *imagedata, unsigned *width, unsigned *height, unsigned *components);
void writeJPEG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned components);

#ifdef __cplusplus
}
#endif

#endif

