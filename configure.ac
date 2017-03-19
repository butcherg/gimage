AC_PREREQ([2.68])
AC_INIT(gimage, 0.2)

AC_LANG([C++])
: ${CXXFLAGS=""}
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

#Process enables:
AC_ARG_ENABLE([openmp],
	AS_HELP_STRING([--enable-openmp], [enables OpenMP threading (-fopenmp in compile and link)])
)
if test "$enable_openmp" == "yes"
then
	CXXFLAGS="$CXXFLAGS -fopenmp"
	LDFLAGS="$LDFLAGS -fopenmp"
fi


AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug], [enables debug compilation with -g, default is no debug switch])
)
if test "$enable_debug" == "yes"
then
	CXXFLAGS="$CXXFLAGS -g"
fi


#Process withs:

AC_ARG_WITH([optimization],
	AS_HELP_STRING([--with-optimization], [enables -O4 optimization, default is -O2])
)
if test "$with_optimization" == "yes"
then
	CXXFLAGS="$CXXFLAGS -O4"
else
	CXXFLAGS="$CXXFLAGS -O2"
fi


AC_ARG_WITH([pixtype],
	AS_HELP_STRING([--with-pixtype], [=half|float|double, specifies data type for pixel structure, default is float])
)
if test "$with_pixtype" != ""
then
	CPPFLAGS="$CPPFLAGS -DPIXTYPE=$with_pixtype"
fi

AC_ARG_WITH([localprefix],
	AS_HELP_STRING([--with-localprefix], [=PATH, specifies path to a non-standard location of lib and include directories.  The lib and include paths are prepended to LDFLAGS and CPPFLAGS, respectively.  Use this to make and use alternate combinations of the supporting libraries])
)
if test "$with_localprefix" != ""
then
	if test "$with_localprefix" != "yes"
		CPPFLAGS="-I$with_localprefix/include $CPPFLAGS"
		LDFLAGS="-L$with_localprefix/lib $LDFLAGS"
	fi
fi

AC_ARG_WITH([jpeg],
	AS_HELP_STRING([--with-jpeg], [=PATH, specifies path to non-standard location of jpeg library.  Omitting the path looks for a jpeg-* directory in the cwd.])
)
if test "$with_jpeg" == "yes"  #look for unpacked jpeg library in ./
then
	JPEG=`stat --printf='%n' jpeg-* 2>/dev/null`
	if test $JPEG == ""  
	then 
		AC_ERROR([no jpeg library found in ./]) 
	fi
	if test -d $JPEG
	then
		CPPFLAGS="$CPPFLAGS -I$JPEG"
		LDFLAGS="$LDFLAGS -L$JPEG"
	else
		AC_ERROR([thing found is not a jpeg library in ./])
	fi
else
	if test "$with_jpeg" != ""
	then
		CPPFLAGS="$CPPFLAGS -I$with_jpeg"
		LDFLAGS="$LDFLAGS -L$with_jpeg"
	fi
fi


AC_ARG_WITH([tiff],
	AS_HELP_STRING([--with-tiff], [=PATH, specifies path to non-standard location of tiff library.  Omitting the path looks for a tiff* directory in the cwd.])
)
if test "$with_tiff" == "yes"  #look for unpacked tiff library in ./
then
	TIFF=`stat --printf='%n' tiff-* 2>/dev/null`
	if test $TIFF == ""  
	then 
		AC_ERROR([no tiff library found in ./]) 
	fi
	if test -d $TIFF
	then
		CPPFLAGS="$CPPFLAGS -I$TIFF/libtiff"
		LDFLAGS="$LDFLAGS -L$TIFF/libtiff/.libs"
	else
		AC_ERROR([thing found is not a tiff library in ./])
	fi
else
	if test "$with_tiff" != ""
	then
		CPPFLAGS="$CPPFLAGS -I$with_tiff/libtiff"
		LDFLAGS="$LDFLAGS -L$with_tiff/libtiff/.libs"
	fi
fi


AC_ARG_WITH([lcms2],
	AS_HELP_STRING([--with-lcms2], [=PATH, specifies path to non-standard location of Little CMS2 library.  Omitting the path looks for a lcms2* directory in the cwd.])
)
if test "$with_lcms2" == "yes"  #look for unpacked tiff library in ./
then
	LCMS2=`stat --printf='%n' lcms2-* 2>/dev/null`
	if test $LCMS2 == ""  
	then 
		AC_ERROR([no tiff library found in ./]) 
	fi
	if test -d $LCMS2
	then
		CPPFLAGS="$CPPFLAGS -I$LCMS2/include"
		LDFLAGS="$LDFLAGS -L$LCMS2/src/.libs"
	else
		AC_ERROR([thing found is not a lcms2 library in ./])
	fi
else
	if test "$with_lcms2" != ""
	then
		CPPFLAGS="$CPPFLAGS -I$with_lcms2/include"
		LDFLAGS="$LDFLAGS -L$with_lcms2/src/.libs"
	fi
fi


AC_ARG_WITH([libraw],
	AS_HELP_STRING([--with-libraw], [=PATH, specifies path to non-standard location of LibRaw library. Omitting the path looks for a Libraw* directory in the cwd.])
)
if test "$with_libraw" == "yes"  #look for unpacked tiff library in ./
then
	LIBRAW=`stat --printf='%n' LibRaw-* 2>/dev/null`
	if test $LIBRAW == ""  
	then 
		AC_ERROR([no LibRaw library found in ./]) 
	fi
	if test -d $LIBRAW
	then
		CPPFLAGS="$CPPFLAGS -I$LIBRAW"
		LDFLAGS="$LDFLAGS -L$LIBRAW/lib/.libs"
	else
		AC_ERROR([thing found is not a libraw library in ./])
	fi
else
	if test "$with_libraw" != ""
	then
		CPPFLAGS="$CPPFLAGS -I$with_libraw"
		LDFLAGS="$LDFLAGS -L$with_libraw/lib/.libs"
	fi
fi


# Checks for libraries.

#AC_SEARCH_LIBS(socket,ws2_32, [], [AC_ERROR([A working ws2_32 is required])])
AC_CHECK_LIB([ws2_32],[main])

AC_CHECK_HEADERS(jpeglib.h, [], [AC_ERROR([jpeglib.h not found])])
AC_SEARCH_LIBS(jpeg_set_defaults,jpeg, [], [AC_ERROR([A working libjpeg is required])])

AC_CHECK_HEADERS(tiffio.h, [], [AC_ERROR([tiffio.h not found])])
AC_SEARCH_LIBS(TIFFSetErrorHandler,tiff, [], [AC_ERROR([A working libtiff is required])])

AC_CHECK_HEADERS(lcms2.h, [], [AC_ERROR([lcms2.h not found])])
AC_SEARCH_LIBS(cmsSetLogErrorHandler,lcms2, [], [AC_ERROR([A working libcms2 is required])])

AC_CHECK_HEADERS(libraw/libraw.h, [], [AC_ERROR([libraw.h not found])])
AC_SEARCH_LIBS(libraw_init,raw, [], [AC_ERROR([A working libraw is required])])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdint.h stdlib.h string.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime floor memmove memset mkdir modf pow rint sqrt strchr strstr utime])


AC_OUTPUT(Makefile)

