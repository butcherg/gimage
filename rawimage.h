#ifndef _rawimage_h
#define _rawimage_h

#ifdef __cplusplus
extern "C" {
#endif

char * loadRAW(const char *filename, char *imagedata, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits);

#ifdef __cplusplus
}
#endif

#endif

