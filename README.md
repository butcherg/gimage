# gimage
Simple image loader/reader/manipulator for raw, jpeg, tiff, and png

This library was written to replace FreeImage in my raw image processing program, rawproc.  I was generally happy with FreeImage,
but I wanted to take advantage of the full range of options in LibRaw, the library FreeImage uses to open camera raw images, and 
some of the inherent organization of FreeImage made that challenging.  That, and incorporation of color managment pushed me over
the edge, and over the period of just under a year I wrote gimage.

gimage makes no pretense of being a general purpose image library; it only opens camera raws, jpegs and tiffs, and only saved jpegs
and tiffs.  I might later add pngs, but it's not very high on the priority list.  It also only includes the image operations that 
interest me, supporting a workflow that produces web-ready images.  All that said, I think you would find gimage.cpp to be a useful
reference to basic image processing, as I've captured a year's worth of learning and research in producing C++ versions of the 
essential operations: curve manipulation, rotate, resize, crop, sharpen, denoise (nlmeans, at that), and a few others one might 
find useful.

A unique 'feature' is the arbitrary conversion of any input image to floating point, 0.0 - 1.0.  All tone manipulation past the 
linear data set recorded by the camera starts to separate tones, you just don't see it in the 16-bit range until the separation 
magnitude becomes > 256, the range of the jpeg you're usually looking at.  Manipulating in floating point helps to postpone
reaching the visible threshold.  It also makes the manipulation algorithms easier to read.

Probably the most powerful feature is the exposure of every single procesing parameter made available by LibRaw. When opening a
raw file, you can load the params with key=value pairs corresponding to any of the Libraw param fields, and the values will
be loaded into the corresponding LibRaw fields and passed to the processor.  I've also abstracted a few of the parameters to 
a more understandable format, for instance, colorspace can be linear|srgb|wide|prophoto|xyz.  As of this writing, LibRaw 0.17.2 
is represented; 0.18.2 is targeted for later in 2017.

If you're going to compile rawproc, you need to first build gimage, so here's how to approach it:

1. Prerequisites:

gimage currently requires libjpeg, libtiff, libcms2, and libraw.  If you're building for Linux, you can just use the distro offerings of
libjpeg and libtiff.  For the others in both Linux and Windows, I advise you to build them from sources, as the distro versions don't seem
to keep up.  gimage has a autoconf-based configure script that supports a few options regarding libraries; if you install all of the four 
required libraries in standard locations, you can do a simple ./configure && make and you'll get a libgimage.a (I prefer static linking).

2. Organization:

The above said, here's how I organize my building.  Note that, in addition to a native Linux build, I also cross-compile win32 and win64
images with mingw-w64, so this may be overkill for you.  Anyway...

<UL>
<LI>Make a top-level directory.</LI>
<LI>In that directory, unpack or clone those of the four prerequisite library sources you choose, as well as gimage.</LI>
<LI>In each, make a build/ directory, cd to it, and run ../configure, and make.</LI>
 </UL>
 
For guidance, here are my ../configures for each, noting that I am using the distro libjpeg and libtiff in their standard places:

<pre>
#LitteCMS2:
../configure --disable-shared

#LibRaw:
../configure --enable-openmp --enable-jpeg --enable-lcms --enable-demosaic-pack-gpl2 --enable-demosaic-pack-gpl3 --disable-jasper
--disable-examples --disable-shared LCMS2_CFLAGS=-I../../lcms2-2.8/include LCMS2_LIBS=../../lcms2-2.8/build/src/.libs

#gimage:
../configure --enable-openmp --enable-optimization --with-lcms2=../../lcms2-2.8 --with-libraw=../../LibRaw-0.18.2 BUILDDIR=build
</pre>

Note that I used LibRaw's environment variables to pluck the lcms2 .h and .a files from its source directory.  Also, gimage has
--withs for all four libraries to do the same thing.  Also note the BUILDDIR variable; gimage's configure uses that to reference
the appropriate build directory in each library's source directory, so YOU NEED TO USE THE SAME DIRECTORY NAME IN ALL THE LIBRARIES
YOU BUILD.  Mine was 'build'; I intend to use 'win32' and win64 for the cross-compiles.

gimage also has a --with-localprefix option to help folks using MinGW on Windows.  This option allows you to specify a root 
directory in which local/lib local/include can be found with all the requisite libraries, in which case you'd install all of
the prerequisite libraries you compile with 'make install DESTDIR=path', and specify --with-localprefix=path when configuring gimage.
In this scheme, you can also install gimage the same way, and have all your libraries in one place when you go to build rawproc.

Hope that's enough to get one started.

Glenn Butcher
4/22/2017
