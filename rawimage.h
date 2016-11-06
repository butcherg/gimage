#ifndef _rawimage_h
#define _rawimage_h

#include <string>
#include <map>

char * _loadRAW_m(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params="",
			char * icc_m=NULL, 
			unsigned  *icclength=0);

#endif

