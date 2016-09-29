#ifndef _jpegimage_h
#define _jpegimage_h

#ifdef __cplusplus
extern "C" {
#endif

void loadJPEG(const char *filename, void *imagedata, unsigned *width, unsigned *height);
void writeJPEG(const char *filename, void *imagedata, unsigned *width, unsigned *height);

#ifdef __cplusplus
}
#endif

#endif