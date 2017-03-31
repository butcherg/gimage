OBJDIR=build
SYS=$(shell gcc -dumpmachine)

CC=gcc
CXX=g++

#+= these in $(OBJDIR) for site-specific things
LIBDIRS=-L$(OBJDIR)/lib
LIBS=-lgimage -lraw -ltiff -ljpeg -llcms2
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
	mkdir -p $(OBJDIR)/include/gimage
	cp gimage.h $(OBJDIR)/include/gimage
	cp curve.h $(OBJDIR)/include/gimage
	cp strutil.h $(OBJDIR)/include/gimage
	cp half.hpp $(OBJDIR)/include/gimage
	rm -f $(OBJDIR)/lib/libgimage.a
	ar rcs $@ $(LIBOBJECTS)
	ranlib $@
	echo "gimage:" > $(OBJDIR)/build.txt
	#git status |grep "On branch" >> $(OBJDIR)/build.txt
	echo "CFLAGS=$(CFLAGS)" >> $(OBJDIR)/build.txt
	echo "LFLAGS=$(LFLAGS)" >> $(OBJDIR)/build.txt

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
	rm -f $(OBJDIR)/gimg$(EXT) $(OBJDIR)/*.o $(OBJDIR)/lib/libgimage.a





