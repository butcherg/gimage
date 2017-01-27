#ifndef _jpegimage_h
#define _jpegimage_h

bool _loadJPEGInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			std::map<std::string,std::string> &info,
			std::string params="",
			char ** iccprofile=NULL, 
			unsigned  *icclength=0);

char * _loadJPEG(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			std::map<std::string,std::string> &info,
			std::string params="",
			char ** iccprofile=NULL, 
			unsigned  *icclength=0);

void _writeJPEG(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			std::map<std::string,std::string> info,
			std::string params="",
			char * iccprofile=NULL, 
			unsigned iccprofilelength=0);


#endif


/*
char * _loadTIFF(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params="",
			char ** icc_m=NULL, 
			unsigned  *icclength=0);

void _writeTIFF(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			unsigned numbits, 
			std::map<std::string,std::string> info, 
			char *iccprofile=NULL, 
			unsigned iccprofilelength=0);
*/
