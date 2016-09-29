#include "jpegimage.h"

int main (int argc, char **argv)
{
	unsigned width, height;
	char i[256];
	if (argc >=2) loadJPEG(argv[1], i, &width, &height);

}

