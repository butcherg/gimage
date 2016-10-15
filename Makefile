OBJDIR=build
SYS=$(shell gcc -dumpmachine)

CC=gcc
CXX=g++

#+= these in $(OBJDIR) for site-specific things
LIBDIRS=
LIBS=-lraw -ltiff -ljpeg 
INCLUDEDIRS=
CFLAGS=-fopenmp -O4
LFLAGS=-fopenmp

-include $(OBJDIR)/localmake.txt

OBJECTS := $(addprefix $(OBJDIR)/,nef2jpg.o gimage.o jpegimage.o jpegexif.o rawimage.o tiffimage.o elapsedtime.o)


$(OBJDIR)/nef2jpg: $(OBJECTS)
	$(CXX) $(LFLAGS) -o$@  $(OBJECTS) $(LIBDIRS) $(LIBS)


$(OBJDIR)/nef2jpg.o: nef2jpg.cpp
	$(CC) $(CFLAGS) $(INCLUDEDIRS) -c nef2jpg.cpp -o$@

$(OBJDIR)/gimage.o: gimage.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -O4 -fopenmp -c gimage.cpp -o$@

$(OBJDIR)/tiffimage.o: tiffimage.cpp tiffimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c tiffimage.cpp -o$@

$(OBJDIR)/rawimage.o: rawimage.cpp rawimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS)   -c rawimage.cpp -o$@

$(OBJDIR)/jpegimage.o: jpegimage.c jpegimage.h
	$(CC) $(CFLAGS) $(INCLUDEDIRS) -c jpegimage.c -o$@

$(OBJDIR)/jpegexif.o: jpegexif.cpp jpegexif.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegexif.cpp -o$@

$(OBJDIR)/elapsedtime.o: elapsedtime.cpp elapsedtime.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c elapsedtime.cpp -o$@

clean:
ifeq ($(SYS), mingw32)
	del $(OBJDIR)/nef2jpg.exe $(OBJDIR)/*.o
else
	rm -f $(OBJDIR)/nef2jpg $(OBJDIR)/*.o
endif

