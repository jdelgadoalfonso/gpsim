#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([gpsim], [0.22.0], [<URL:http://www.dattalo.com/gnupic/gpsim.html>])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

# Determine the host and build type. The target is always a PIC.
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

dnl --disable-gui : turn off gui support (cli only)
dnl    The default is to have the gui. 

AC_ARG_ENABLE(gui,
     [  --disable-gui           Only use the cli and not the gui],
     [case "${enableval}" in
       yes) use_gui=yes ;;
       no)  use_gui=no ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --disable-gui) ;;
     esac],[use_gui=yes])

if test "$use_gui" = "no"; then
	echo disabling gui support
else
	echo enabling gui support
	[
	 use_gui=yes
	]
	AC_DEFINE([HAVE_GUI],[],[True if GUI is being used])
fi


dnl --enable-sockets : allows gpsim to be controlled from a socket interface
dnl    The default is to not use sockets.

AC_ARG_ENABLE(sockets,
     [  --enable-sockets        Allows gpsim to be controlled via a socket interface],
     [case "${enableval}" in
       yes) use_sockets=yes ;;
       no)  use_sockets=no ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-sockets) ;;
     esac],[use_sockets=no])

if test "$use_sockets" = "no"; then
        echo disabling gpsim socket interface
else
        echo enabling gpsim socket interface
        [
         use_sockets=yes
        ]
	AC_DEFINE([HAVE_SOCKETS],[],[True if gpsim socket interface is being used])
fi

GTK=
GDK=
GLIB=

AC_PATH_PROG(PKGCONFIG, pkg-config, no, ${PATH})

if test $PKGCONFIG = "no"; then
  AC_MSG_ERROR(Cannot find pkg-config)
fi

if test "$use_gui" = "no"; then
  dnl glib2 checks

  X_LDFLAGS=`$PKGCONFIG --libs glib-2.0 gthread-2.0 gmodule-2.0`
  X_CFLAGS=`$PKGCONFIG --cflags glib-2.0`
  Y_LDFLAGS=
  Y_CFLAGS=
else
  dnl gtk2 checks

  $PKGCONFIG --exists gtkextra-2.0
  if test $? != 0; then
    AC_MSG_ERROR(Cannot find gtkextra-2.0 package)
  fi

  X_LDFLAGS=`$PKGCONFIG --libs gtk+-2.0 gthread-2.0`
  X_CFLAGS=`$PKGCONFIG --cflags gtk+-2.0`
  Y_LDFLAGS=`$PKGCONFIG --libs gtkextra-2.0`
  Y_CFLAGS=`$PKGCONFIG --cflags gtkextra-2.0`
  GTK_VERSION_T=`$PKGCONFIG --modversion gtk+-2.0`
  echo linking with gtk-$GTK_VERSION_T
  AC_DEFINE_UNQUOTED([GTK_VERSION],"$GTK_VERSION_T",[gtk version])
fi

wi_LIB_READLINE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_YACC
AM_PROG_LEX
AM_PROG_LIBTOOL

# Checks for libraries.
AC_LANG([C++])
#AC_CHECK_LIB([gpsim], [main])
AC_CHECK_LIB([popt], [main])
#AC_CHECK_LIB([pthread], [main])


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_TYPE_SIGNAL

# Checks for library functions.
AC_FUNC_ALLOCA
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor gethostbyname gethostname gettimeofday memset pow select socket sqrt strcasecmp strchr strdup strerror strncasecmp strndup strpbrk strrchr strstr strtoul])

# printf modifier define for long long as "ll"
# config_win32.h.in defines this for Visual Studio stdclib as "I64"
AC_DEFINE([PRINTF_INT64_MODIFIER],["ll"],[printf modifier define for long long])


AM_CFLAGS=
AM_CXXFLAGS=
AM_LDFLAGS=

# Options for the system on which the package will run
case "${host}" in
  *linux* )
    if test "x$GCC" = "xyes"; then
      AM_CFLAGS="-Wall"
      AM_CXXFLAGS="-Wall"
      AM_LDFLAGS="-Wl,-warn-common -Wl,-warn-once"
    fi
    ;;
  *mingw* )
    ;;
esac

# Host filesystem options
case "${host}" in
  *mingw* | *-pc-os2_emx | *-pc-os2-emx | *djgpp* )
    AC_DEFINE(HAVE_DOS_BASED_FILE_SYSTEM, 1, 
      [Define if your host uses a DOS based file system. ])
    ;;
esac

AC_SUBST(Y_CFLAGS)
AC_SUBST(Y_LDFLAGS)
AC_SUBST(X_CFLAGS)
AC_SUBST(X_LDFLAGS)
AC_SUBST(GTK)
AC_SUBST(GDK)
AC_SUBST(GLIB)
AC_SUBST(LIBREADLINE)

AC_CONFIG_FILES([Makefile
                 cli/Makefile
                 doc/Makefile
                 examples/Makefile
                 examples/modules/Makefile
                 examples/projects/Makefile
                 eXdbm/Makefile
                 gpsim/Makefile
                 gui/Makefile
                 modules/Makefile
		 regression/Makefile
                 src/Makefile
		 src/dspic/Makefile
                 xpms/Makefile
                 gpsim.spec])

AC_OUTPUT

AC_MSG_RESULT([

gpsim-$PACKAGE_VERSION is now configured for $canonical_host_type

  Build:                $build
  Host:                 $host
  Source directory:     $srcdir
  Installation prefix:  $prefix
  C compiler:           $CC $CPPFLAGS $CFLAGS
  C++ compiler:         $CXX $CPPFLAGS $CXXFLAGS

  gui:                  $use_gui
  Socket interface:     $use_sockets

])
