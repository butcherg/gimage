

LIBS=-Ljpeg-6b/ -LLibRaw-0.17.2/lib/.libs -Ltiff-4.0.6/libtiff/.libs
INCLUDES=-Ijpeg-6b -ILibRaw-0.17.2/libraw -Itiff-4.0.6/libtiff

LIBRAW_FLAGS=-DLIBRAW_NODLL


nef2jpg: nef2jpg.o gimage.o jpegimage.o rawimage.o tiffimage.o
	$(CXX) -fopenmp -o nef2jpg  nef2jpg.o jpegimage.o rawimage.o tiffimage.o gimage.o   $(LIBS) -lraw -ltiff -ljpeg -ljasper -llcms2 -lm -lstdc++  -llzma -ljbig -lz


nef2jpg.o: nef2jpg.cpp
	$(CC) $(INCLUDES) -o nef2jpg.o -c nef2jpg.cpp

gimage.o: gimage.cpp
	$(CXX) $(INCLUDES) -o gimage.o -c gimage.cpp

tiffimage.o: tiffimage.c tiffimage.h
	$(CC) $(LIBRAW_FLAGS) $(INCLUDES) -o tiffimage.o -c tiffimage.c

rawimage.o: rawimage.cpp rawimage.h
	$(CXX) $(INCLUDES) -fopenmp -o rawimage.o -c rawimage.cpp

jpegimage.o: jpegimage.c jpegimage.h
	$(CC) $(INCLUDES) -o jpegimage.o -c jpegimage.c

clean:
ifeq ($(OS), windows_NT)
	rm -f nef2jpg.exe *.o
else
	rm -f nef2jpg *.o
endif

