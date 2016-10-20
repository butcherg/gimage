#ifndef _jpegexif_h_
#define _jpegexif_h_

#include <string>
#include <map>


void parse_APP1marker(unsigned char * marker, unsigned length, std::map<std::string,std::string> &imageinfo);
unsigned char * construct_APP1marker(std::map<std::string,std::string> imageinfo, unsigned *markerlen);


#endif