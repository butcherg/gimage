AC_PREREQ([2.69])
AC_INIT(gimage, 0.2)

AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

#AC_SUBST([AM_CXXFLAGS], [-fopenmp -O4])

#Process withs:
AC_ARG_WITH([jpeg],
	AS_HELP_STRING([--with-jpeg], [=PATH, specifies path to non-standard location of jpeg library])
)
if test "$with_jpeg" != ""
then
	CXXFLAGS="$CXXFLAGS -I$with_jpeg"
	LDFLAGS="$LDFLAGS -L$with_jpeg"
fi

AC_ARG_WITH([tiff],
	AS_HELP_STRING([--with-tiff], [=PATH, specifies path to non-standard location of tiff library])
)
if test "$with_tiff" != ""
then
	CXXFLAGS="$CXXFLAGS -I$with_tiff/libtiff"
	LDFLAGS="$LDFLAGS -L$with_tiff/libtiff/.libs"
fi

AC_ARG_WITH([lcms2],
	AS_HELP_STRING([--with-lcms2], [=PATH, specifies path to non-standard location of Little CMS2 library])
)
if test "$with_lcms2" != ""
then
	CXXFLAGS="$CXXFLAGS -I$with_lcms2/include"
	LDFLAGS="$LDFLAGS -L$with_lcms2/src/.libs"
fi

AC_ARG_WITH([libraw],
	AS_HELP_STRING([--with-libraw], [=PATH, specifies path to non-standard location of LibRaw library])
)
if test "$with_libraw" != ""
then
	CXXFLAGS="$CXXFLAGS -I$with_libraw/libraw"
	LDFLAGS="$LDFLAGS -L$with_libraw/lib/.libs"
fi

# Checks for libraries.
AC_CHECK_HEADERS(jpeglib.h, [], [AC_ERROR([jpeglib.h not found])])
AC_SEARCH_LIBS(jpeg_std_error,jpeg, [], [AC_ERROR([A working libjpeg is required])])

AC_CHECK_HEADERS(tiffio.h, [], [AC_ERROR([tiffio.h not found])])
AC_SEARCH_LIBS(TIFFSetErrorHandler,tiff, [], [AC_ERROR([A working libtiff is required])])

AC_CHECK_HEADERS(lcms2.h, [], [AC_ERROR([lcms2.h not found])])
AC_SEARCH_LIBS(cmsSetLogErrorHandler,lcms2, [], [AC_ERROR([A working libcms2 is required])])

AC_CHECK_HEADERS(libraw/libraw.h, [], [AC_ERROR([libraw.h not found])])
AC_SEARCH_LIBS(libraw_strerror,raw, [], [AC_ERROR([A working libraw is required])])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdint.h stdlib.h string.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([clock_gettime floor memmove memset mkdir modf pow rint sqrt strchr strstr utime])


AC_OUTPUT(Makefile)

