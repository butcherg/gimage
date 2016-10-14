OBJDIR=build

CC=gcc

CXX=g++

LIBS=-Ljpeg-6b/ -LLibRaw-0.17.2/lib/.libs -Ltiff-4.0.6/libtiff/.libs
INCLUDES=-Ijpeg-6b -ILibRaw-0.17.2/libraw -Itiff-4.0.6/libtiff

LIBRAW_FLAGS=-DLIBRAW_NODLL

WINLIBS=-lws2_32
WINLINKFLAGS=-static
LINLIBS=-ljasper -llcms2 -lm -lstdc++  -llzma -ljbig -lz

OBJECTS := $(addprefix $(OBJDIR)/,nef2jpg.o gimage.o jpegimage.o rawimage.o tiffimage.o elapsedtime.o)


$(OBJDIR)/nef2jpg: $(OBJECTS)
	$(CXX) -fopenmp  -o$@  $(OBJECTS)   $(LIBS) -lraw -ltiff -ljpeg $(LINLIBS)


$(OBJDIR)/nef2jpg.o: nef2jpg.cpp
	$(CC) $(INCLUDES) -c nef2jpg.cpp -o$@

$(OBJDIR)/gimage.o: gimage.cpp
	$(CXX) $(INCLUDES) -O4 -fopenmp -c gimage.cpp -o$@

$(OBJDIR)/tiffimage.o: tiffimage.cpp tiffimage.h
	$(CXX) $(INCLUDES)  -c tiffimage.cpp -o$@

$(OBJDIR)/rawimage.o: rawimage.cpp rawimage.h
	$(CXX) $(LIBRAW_FLAGS) $(INCLUDES) -fopenmp  -c rawimage.cpp -o$@

$(OBJDIR)/jpegimage.o: jpegimage.c jpegimage.h
	$(CC) $(INCLUDES)  -c jpegimage.c -o$@

$(OBJDIR)/elapsedtime.o: elapsedtime.cpp elapsedtime.h
	$(CXX) $(INCLUDES)  -c elapsedtime.cpp -o$@

clean:
ifeq ($(OS), windows_NT)
	rm -f $(OBJDIR)/nef2jpg.exe $(OBJDIR)/*.o
else
	rm -f $(OBJDIR)/nef2jpg $(OBJDIR)/*.o
endif

