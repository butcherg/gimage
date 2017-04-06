AC_PREREQ([2.68])
AC_INIT(gimage, 0.2)



# MINGW_AC_WIN32_NATIVE_HOST
# --------------------------
# Check if the runtime platform is a native Win32 host.
#
AC_DEFUN([MINGW_AC_WIN32_NATIVE_HOST],
[AC_CACHE_CHECK([whether we are building for a Win32 host], 
                [mingw_cv_win32_host],
AC_COMPILE_IFELSE([AC_LANG_SOURCE([
#ifdef _WIN32
choke me
#endif
])], [mingw_cv_win32_host=no], [mingw_cv_win32_host=yes]))])


AC_LANG([C++])
#: ${CXXFLAGS=""}
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

MINGW_AC_WIN32_NATIVE_HOST

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


AC_ARG_ENABLE([optimization],
	AS_HELP_STRING([--enable-optimization], [enables -O4 optimization, default is -O2])
)
if test "$enable_optimization" == "yes"
then
	CXXFLAGS="$CXXFLAGS -O4"
else
	CXXFLAGS="$CXXFLAGS -O2"
fi

#Process withs:


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
	then
		CPPFLAGS="-I$with_localprefix/usr/local/include $CPPFLAGS"
		LDFLAGS="-L$with_localprefix/usr/local/lib $LDFLAGS"
	fi
fi

#AC_SEARCH_LIBS(socket,ws2_32, [], [AC_ERROR([A working ws2_32 is required])])

#if test "$IS_WIN32" == "yes" ;
if test "$mingw_cv_win32_host" == "yes" ;
then
#	AC_MSG_NOTICE ([adding win32 libs and flags])
	AC_CHECK_LIB([ws2_32],[main])
	CXXFLAGS="$CXXFLAGS -DLIBRAW_NODLL"
fi

MINGW_AC_WIN32_NATIVE_HOST

AC_ARG_WITH([jpeg],
	AS_HELP_STRING([--with-jpeg], [=PATH, specifies path to non-standard location of the jpeg library.]),
	[
		echo "libjpeg: using $with_jpeg"
		if test -d $with_jpeg 
		then
			CPPFLAGS=" -I$with_jpeg $CPPFLAGS"
			LIBS="$with_jpeg/libjpeg.a $LIBS"
		else
			AC_ERROR([no jpeg directory found.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(jpeglib.h, [], [AC_ERROR([jpeglib.h not found.])])
		AC_SEARCH_LIBS(jpeg_set_defaults,jpeg, [], [AC_ERROR([A working libjpeg is required.])])
	]
)


AC_ARG_WITH([tiff],
	AS_HELP_STRING([--with-tiff], [=PATH, specifies path to non-standard location of the tiff library.]),
	[
		echo "libtiff: using $with_tiff"
		if test -d $with_tiff 
		then
			CPPFLAGS="-I$with_tiff/libtiff $CPPFLAGS"
			LIBS="$with_tiff/libtiff/.libs/libtiff.a $LIBS"
		else
			AC_ERROR([no tiff directory found.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(tiffio.h, [], [AC_ERROR([tiffio.h not found.])])
		AC_SEARCH_LIBS(TIFFSetErrorHandler,tiff, [], [AC_ERROR([A working libtiff is required.])])
	]
)


AC_ARG_WITH([libraw],
	AS_HELP_STRING([--with-libraw], [=PATH, specifies path to non-standard location of the LibRaw library.]),
	[
	echo "libraw: using $with_libraw"
		if test -d $with_libraw 
		then
			CPPFLAGS="-I$with_libraw $CPPFLAGS"
			LIBS="$with_libraw/lib/.libs/libraw.a $LIBS"
		else
			AC_ERROR([no libraw directory found.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(libraw/libraw.h, [], [AC_ERROR([libraw.h not found])])
		AC_SEARCH_LIBS(libraw_init,raw, [], [AC_ERROR([A working libraw is required])])
	]
)


AC_ARG_WITH([lcms2],
	AS_HELP_STRING([--with-lcms2], [=PATH, specifies path to non-standard location of Little CMS2 library.]),
	[
	echo "libcms2: using $with_lcms2"
		if test -d $with_lcms2 
		then
			CPPFLAGS="-I$with_lcms2/include $CPPFLAGS "
			LIBS="$with_lcms2/src/.libs/liblcms2.a $LIBS "
		else
			AC_ERROR([no lcms2 directory found.]) 
		fi
	],
	[
		AC_CHECK_HEADERS(lcms2.h, [], [AC_ERROR([lcms2.h not found])])
		AC_SEARCH_LIBS(cmsSetLogErrorHandler,lcms2, [], [AC_ERROR([A working libcms2 is required])])
	]
)

CPPFLAGS="-I$with_lcms2/include $CPPFLAGS "


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

