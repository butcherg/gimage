
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main (int argc, char **argv)
{
	unsigned width, height, numcolors, numbits, x, y;
	gImage * image = NULL;
	gImage *sharperimage = NULL;
	
	char loadext[3], saveext[3];
	strncpy(loadext,argv[1]+strlen(argv[1])-3,3);
	if (argc >= 3) strncpy(saveext,argv[2]+strlen(argv[2])-3,3);


	if (argc >=2) {
		printf("loading %s...\n",argv[1]);
		if (strcmp(loadext,"tif") == 0) 
			image = gImage::loadTIFF(argv[1]);
		else if (strcmp(loadext,"NEF") == 0) 
			image = gImage::loadRAW(argv[1]);
		else if (strcmp(loadext,"jpg") == 0)
			image = gImage::loadJPEG(argv[1]);
		else exit(1);
	}

	std::map<std::string,std::string> imginfo = image->getInfo();

	for (std::map<std::string,std::string>::iterator it=imginfo.begin(); it!=imginfo.end(); ++it)
		printf("%s: %s\n",it->first.c_str(), it->second.c_str());
	printf("\n");

	
	sharperimage = image->Sharpen(1,1);
	//sharperimage = image->Copy();

	if (argc >=3) {
		printf("saving %s...\n",argv[2]);
		if (strcmp(saveext,"jpg") == 0) {
			sharperimage->saveJPEG(argv[2]);
		}
		if (strcmp(saveext,"tif") == 0) {
			sharperimage->saveTIFF(argv[2], 16);
			//image->saveTIFF(argv[2], 16);
		}

	}

	if (image) image->~gImage(); 
	if (sharperimage) sharperimage->~gImage();

}

