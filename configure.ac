AC_INIT([gTimelapse],[1.0], [timnugent@gmail.com])
AC_CONFIG_SRCDIR(src/gtimelapse.cpp)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([config])

AM_INIT_AUTOMAKE([-Wall])

AC_PROG_CXX
AC_PROG_LIBTOOL

m4_include([m4/gp-check-library.m4])
GP_CHECK_LIBRARY([LIBGPHOTO2], [libgphoto2], [>= 2.2.1])

AM_OPTIONS_WXCONFIG
reqwx=2.8.9
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1; then
	AC_MSG_ERROR([
		wxWidgets must be installed on your system.
 
		Please check that wx-config is in path, the directory
		where wxWidgets libraries are installed (returned by
		'wx-config --libs' or 'wx-config --static --libs' command)
		is in LD_LIBRARY_PATH or equivalent variable and
		wxWidgets version is $reqwx or above.
		])
fi
 
CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"

AC_OUTPUT(Makefile src/Makefile)








