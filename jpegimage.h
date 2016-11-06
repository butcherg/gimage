#ifndef _jpegimage_h
#define _jpegimage_h

char * _loadJPEG(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			std::map<std::string,std::string> &info,
			std::string params="",
			char * icc_m=NULL, 
			unsigned  *icclength=0);

void _writeJPEG(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			std::map<std::string,std::string> info,
			std::string params="",
			char * icc_m=NULL, 
			unsigned  *icclength=0);


#endif

