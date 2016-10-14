
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "elapsedtime.h"


int main (int argc, char **argv)
{
	unsigned width, height, numcolors, numbits, x, y;
	gImage * image = NULL;
	gImage *sharperimage = NULL;
	
	char loadext[4], saveext[4];
	strncpy(loadext,argv[1]+strlen(argv[1])-3,3); loadext[3] = '\0';
	if (argc >= 3) strncpy(saveext,argv[2]+strlen(argv[2])-3,3); saveext[3] = '\0';


	if (argc >=2) {
		
		if (strcmp(loadext,"tif") == 0) {
			printf("loading %s...\n",argv[1]);
			image = gImage::loadTIFF(argv[1]);
		}
		else if (strcmp(loadext,"NEF") == 0) {
			printf("loading %s...\n",argv[1]);
			image = gImage::loadRAW(argv[1]);
		}
		else if (strcmp(loadext,"jpg") == 0) {
			printf("loading %s...\n",argv[1]);
			image = gImage::loadJPEG(argv[1]);
		}
		else {
			printf ("type of %s not recognized: %s\n",argv[1], loadext); 
			exit(1);
		}
	}

	std::map<std::string,std::string> imginfo = image->getInfo();

	for (std::map<std::string,std::string>::iterator it=imginfo.begin(); it!=imginfo.end(); ++it)
		printf("%s: %s\n",it->first.c_str(), it->second.c_str());
	printf("\n");



	_mark();
	sharperimage = image->Sharpen(1,4);
	printf("gImage::Sharpen: %f sec\n",_duration());
	//sharperimage = image->Copy();

	image->Stats();
	sharperimage->Stats();

	if (argc >=3) {
		printf("saving %s...\n",argv[2]);
		if (strcmp(saveext,"jpg") == 0) {
			sharperimage->saveJPEG(argv[2]);
		}
		else if (strcmp(saveext,"tif") == 0) {
			sharperimage->saveTIFF(argv[2], 16);
			//image->saveTIFF(argv[2], 16);
		}
		else printf("type of %s not recognized.\n",argv[2]);

	}

	if (image) image->~gImage(); 
	if (sharperimage) sharperimage->~gImage();

}

