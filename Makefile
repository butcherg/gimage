OBJDIR=build
SYS=$(shell gcc -dumpmachine)

CC=gcc
CXX=g++

#+= these in $(OBJDIR) for site-specific things
LIBDIRS=-L$(OBJDIR)
LIBS=-lgimage -lraw -ltiff -ljpeg -llcms2
INCLUDEDIRS=-I.
CFLAGS=
LFLAGS=

-include $(OBJDIR)/localmake.txt

OBJECTS := $(addprefix $(OBJDIR)/,gimg.o elapsedtime.o )
LIBOBJECTS := $(addprefix $(OBJDIR)/,gimage.o jpegimage.o jpegexif.o rawimage.o tiffimage.o strutil.o curve.o)

$(OBJDIR)/gimg: $(OBJECTS)  $(OBJDIR)/libgimage.a
	$(CXX) $(LFLAGS) -o$@$(EXT)  $(LIBDIRS) $(OBJECTS) $(LIBS) 

$(OBJDIR)/libgimage.a: $(LIBOBJECTS)
	rm -f $(OBJDIR)/libgimage.a
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

$(OBJDIR)/tiffimage.o: tiffimage.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c tiffimage.cpp -o$@

$(OBJDIR)/rawimage.o: rawimage.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c rawimage.cpp -o$@

$(OBJDIR)/jpegimage.o: jpegimage.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegimage.cpp -o$@

$(OBJDIR)/jpegexif.o: jpegexif.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c jpegexif.cpp -o$@

$(OBJDIR)/elapsedtime.o: elapsedtime.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c elapsedtime.cpp -o$@

$(OBJDIR)/strutil.o: strutil.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c strutil.cpp -o$@

$(OBJDIR)/curve.o: curve.cpp
	$(CXX) $(CFLAGS) $(INCLUDEDIRS) -c -w curve.cpp -o$@

clean:
	rm -f $(OBJDIR)/gimg$(EXT) $(OBJDIR)/*.o $(OBJDIR)/lib/libgimage.a





