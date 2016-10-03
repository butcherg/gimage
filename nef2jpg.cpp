//#include "jpegimage.h"
//#include "rawimage.h"
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main (int argc, char **argv)
{
	unsigned width, height, numcolors, numbits, x, y;
	gImage * image;
	
	char loadext[3], saveext[3];
	strncpy(loadext,argv[1]+strlen(argv[1])-3,3);
	strncpy(saveext,argv[2]+strlen(argv[2])-3,3);


	if (argc >=2) {
		printf("loading %s...\n",argv[1]);
		if (strcmp(loadext,"NEF") == 0) 
			image = gImage::loadRAW(argv[1]);
		else if (strcmp(loadext,"jpg") == 0)
			image = gImage::loadJPEG(argv[1]);
		else exit(1);
	}

	if (argc >=3) {
		printf("saving %s...\n",argv[2]);
		if (strcmp(saveext,"jpg") == 0) {
			image->saveJPEG(argv[2]);
		}
		else {
			image->~gImage();
			exit(1);
		}

	}
	image->~gImage(); 


}

