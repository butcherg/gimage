#ifndef _rawimage_h
#define _rawimage_h

#include <string>
#include <map>

char * _loadRAW_m(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, char * info_m, char * icc_m, unsigned *icclength);

char * _loadRAW1_m(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			char * icc_m, 
			unsigned  *icclength);

#endif

