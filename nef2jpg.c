#include "jpegimage.h"
#include <stdlib.h>

int main (int argc, char **argv)
{
	unsigned width, height, components;
	char *image;
	if (argc >=2) image = loadJPEG(argv[1], image, &width, &height, &components);
	if (argc >=3) writeJPEG(argv[2], image, width, height, components);
	free(image); 
}

