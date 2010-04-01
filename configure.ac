#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.5)
AC_INIT([soundtank], [1.0.2],
        [Jacob Robbins <jrobbins@nymu.net>])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/soundtank_types.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl this is my hack to try to make sure libxml2 headers are present
AC_CHECK_PROG(HAVE_XML2_CONFIG, xml2-config, yes, no)
if test $HAVE_XML2_CONFIG != yes ; then

   AC_MSG_ERROR(Couldn't find program xml2-config. This probably means you don't have the header files for libxml2 installed. You must get these. Project website is www.xmlsoft.org. Debian users apt-get install libxml2-dev )

fi

dnl ===============
dnl Debugging stuff
dnl ===============

AC_ARG_ENABLE(debug, 
[  --enable-debug          Enable debugging (default=no)],, enable_debug=no)

if test "$enable_debug" = yes; then
echo NOTE:     Debug Compile Selected
CFLAGS="-g -Wall"
else
CFLAGS=" -O2 -Wall"
fi

AC_SUBST(SOUNDTANK_CFLAGS)

#CFLAGS=$SOUNDTANK_CFLAGS

# Checks for libraries.

dnl TODO: efence is only used for debugging
dnl FIXME: Replace `main' with a function in `-lefence':
dnl AC_CHECK_LIB([efence], [main])

AC_CHECK_LIB([asound], [snd_pcm_open], , [AC_MSG_ERROR([Couldn't find ALSA soundcard drivers. You must install these first. See www.alsa-project.org or use ALSA modules in linux kernel version 2.6 For Debian users, apt-get install alsa-base libasound2-dev])])

AC_CHECK_LIB([jack], [jack_client_new], , [AC_MSG_ERROR([Couldn't find JACK Audio Connection Kit. You must install this first. Make sure you install the JACK header files as well. Project website is http://jackit.sf.net. For Debian users, apt-get install jackd libjack0.80.0-dev, or a more recent version])])

AC_CHECK_LIB([ncurses], [initscr], , [AC_MSG_ERROR([Couldn't find ncurses library for basic command terminal input and output. You must install this first.])])

AC_CHECK_LIB([popt], [poptGetContext], , [AC_MSG_ERROR([Couldn't find popt library for parseing command arguments. You must install this first.])])

#NOTE: must add ncurses to library flags for readline test, thank you Redhat9
ORIGINAL_LDFLAGS=$LDFLAGS
LDFLAGS=-lncurses $LDFLAGS
export LDFLAGS

AC_CHECK_LIB([readline], [readline], , [AC_MSG_ERROR([Couldn't find GNU readline library for handling terminal interactions. You must install this first.])])

LDFLAGS=$ORIGINAL_LDFLAGS
export LDFLAGS

AC_CHECK_LIB([xml2], [xmlSaveFormatFile], , [AC_MSG_ERROR([Couldn't find libxml2. You must install this first. See www.xmlsoft.org. Debian users apt-get install libxml2])])

AC_CHECK_LIB([pthread], [pthread_create], , [AC_MSG_ERROR([Couldn't find pthread library for multi-threaded unix applications. You must install this first.])])



# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h unistd.h])

AC_CHECK_HEADER([alsa/asoundlib.h], , [AC_MSG_ERROR([Couldn't find alsa header files. You must install this first. Debian users apt-get install alsa-headers])])

AC_CHECK_HEADER([jack/jack.h], , [AC_MSG_ERROR([Couldn't find header files for jack. You must install this first. Debian users apt-get install libjack0.80.0-dev])])

dnl we don't need the ncurses header, just the library because it's used by readline

AC_CHECK_HEADER([popt.h], , [AC_MSG_ERROR([Couldn't find header files for libpopt. You must install this first. Debian users apt-get install libpopt-dev])])

AC_CHECK_HEADER([readline/readline.h], , [AC_MSG_ERROR([Couldn't find header files for GNU readline library. You must install this first. Debian users apt-get install libreadline4-dev])])

dnl I can't check for libxml2 headers because they're hidden behind the xml2-config program, instead we check for that program and hope it's good enough

AC_CHECK_HEADER([pthread.h], , [AC_MSG_ERROR([Couldn't find header files for pthread library. You must install this first. Most distros include this so I don't know what you should do])])



# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset pow setenv sqrt strdup strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
