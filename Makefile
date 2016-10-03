

LIBS=-Ljpeg-6b/
INCLUDES=-Ijpeg-6b/


nef2jpg: nef2jpg.o gimage.o jpegimage.o rawimage.o
	$(CXX) -fopenmp -o nef2jpg  nef2jpg.o jpegimage.o rawimage.o gimage.o   jpeg-6b/libjpeg.a LibRaw-0.17.2/lib/.libs/libraw.a  -ljpeg -ljasper -llcms2 -lm -lstdc++  


nef2jpg.o: nef2jpg.cpp
	$(CC) $(INCLUDES) -o nef2jpg.o -c nef2jpg.cpp

gimage.o: gimage.cpp
	$(CXX) $(INCLUDES) -o gimage.o -c gimage.cpp

rawimage.o: rawimage.cpp rawimage.h
	$(CXX) $(INCLUDES) -fopenmp -o rawimage.o -c rawimage.cpp

jpegimage.o: jpegimage.c jpegimage.h
	$(CC) $(INCLUDES) -o jpegimage.o -c jpegimage.c

clean:
	rm nef2jpg *.o

