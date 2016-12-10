OBJDIR=build
SYS=$(shell gcc -dumpmachine)

CC=gcc
CXX=g++

#+= these in $(OBJDIR) for site-specific things
LIBDIRS=-L$(OBJDIR)/lib
LIBS=-lgimage -lraw -ltiff -ljpeg 
INCLUDEDIRS=
CFLAGS=
LFLAGS=

-include $(OBJDIR)/localmake.txt

OBJECTS := $(addprefix $(OBJDIR)/,gimg.o elapsedtime.o )
LIBOBJECTS := $(addprefix $(OBJDIR)/,gimage.o jpegimage.o jpegexif.o rawimage.o tiffimage.o strutil.o curve.o)

$(OBJDIR)/gimg: $(OBJECTS)  $(OBJDIR)/lib/libgimage.a
	$(CXX) $(LFLAGS) -o$@$(EXT)  $(LIBDIRS) $(OBJECTS) $(LIBS) 

$(OBJDIR)/lib/libgimage.a: $(LIBOBJECTS)
	mkdir -p $(OBJDIR)/lib
	mkdir -p $(OBJDIR)/include
	cp gimage.h $(OBJDIR)/include
	cp curve.h $(OBJDIR)/include
	cp strutil.h $(OBJDIR)/include
	ar rcs $@ $(LIBOBJECTS)
	ranlib $@

$(OBJDIR)/gimg.o: gimg.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c gimg.cpp -o$@

$(OBJDIR)/gimage.o: gimage.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c gimage.cpp -o$@

$(OBJDIR)/tiffimage.o: tiffimage.cpp tiffimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c tiffimage.cpp -o$@

$(OBJDIR)/rawimage.o: rawimage.cpp rawimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c rawimage.cpp -o$@

$(OBJDIR)/jpegimage.o: jpegimage.cpp jpegimage.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegimage.cpp -o$@

$(OBJDIR)/jpegexif.o: jpegexif.cpp jpegexif.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegexif.cpp -o$@

$(OBJDIR)/elapsedtime.o: elapsedtime.cpp elapsedtime.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c elapsedtime.cpp -o$@

$(OBJDIR)/strutil.o: strutil.cpp strutil.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c strutil.cpp -o$@

$(OBJDIR)/curve.o: curve.cpp curve.h
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c -w curve.cpp -o$@

clean:
ifeq ($(SYS), mingw32)
	rm -f $(OBJDIR)/gimg.exe $(OBJDIR)/*.o
else
	rm -f $(OBJDIR)/gimg $(OBJDIR)/*.o
endif

cleanlib:
	rm -f $(OBJDIR)/lib/libgimage.a



