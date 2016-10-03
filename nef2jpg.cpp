//#include "jpegimage.h"
//#include "rawimage.h"
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>


int main (int argc, char **argv)
{
	unsigned width, height, numcolors, numbits, x, y;



	gImage * image;

	if (argc >=2) {
		printf("loading %s...\n",argv[1]);
		//image = gImage::loadRAW(argv[1]);
		image = gImage::loadJPEG(argv[1]);
	}

	if (argc >=2) {
		image->saveJPEG(argv[2]);
		printf("saving %s...\n",argv[2]);

	}
	image->~gImage(); 



/*
	char *image;

	if (argc >= 2) image = gImage::loadRAWImage(argv[1], image, &width, &height, &numcolors, &numbits);
	//if (argc >= 2) image = loadJPEG(argv[1], image, &width, &height, &numcolors);
	if (argc >= 3) writeJPEG(argv[2], image, width, height, numcolors);
*/
}

