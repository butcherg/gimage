OBJDIR=build
SYS=$(shell gcc -dumpmachine)

CC=gcc
CXX=g++

#+= these in $(OBJDIR) for site-specific things
LIBDIRS=-Lbuild/lib
LIBS=-lgimage -lraw -ltiff -ljpeg 
INCLUDEDIRS=
CFLAGS=-fopenmp -O4
LFLAGS=-fopenmp

-include $(OBJDIR)/localmake.txt

OBJECTS := $(addprefix $(OBJDIR)/,gimg.o gimage.o jpegimage.o jpegexif.o rawimage.o tiffimage.o elapsedtime.o strutil.o Curve.o)

LIBOBJECTS := $(addprefix $(OBJDIR)/,gimage.o jpegimage.o jpegexif.o rawimage.o tiffimage.o)


$(OBJDIR)/gimg: $(OBJDIR)/gimg.o  $(OBJDIR)/lib/libgimage.a
	$(CXX) $(LFLAGS) -o$@  $(LIBDIRS) $(OBJDIR)/gimg.o $(LIBS) 

$(OBJDIR)/lib/libgimage.a: $(LIBOBJECTS)
	ar rcs $@ $(LIBOBJECTS)
	ranlib $@

$(OBJDIR)/gimg.o: gimg.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c -w gimg.cpp -o$@

$(OBJDIR)/gimage.o: gimage.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c -w gimage.cpp -o$@

$(OBJDIR)/tiffimage.o: tiffimage.cpp tiffimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c tiffimage.cpp -o$@

$(OBJDIR)/rawimage.o: rawimage.cpp rawimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS)   -c rawimage.cpp -o$@

$(OBJDIR)/jpegimage.o: jpegimage.cpp jpegimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegimage.cpp -o$@

$(OBJDIR)/jpegexif.o: jpegexif.cpp jpegexif.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegexif.cpp -o$@

$(OBJDIR)/elapsedtime.o: elapsedtime.cpp elapsedtime.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c elapsedtime.cpp -o$@

$(OBJDIR)/strutil.o: strutil.cpp strutil.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c strutil.cpp -o$@

$(OBJDIR)/Curve.o: Curve.cpp Curve.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c -w Curve.cpp -o$@

clean:
ifeq ($(SYS), mingw32)
	rm -f $(OBJDIR)/gimg.exe $(OBJDIR)/*.o
else
	rm -f $(OBJDIR)/gimg $(OBJDIR)/*.o
endif

cleanlib:
	rm -f $(OBJDIR)/libgimage.a



