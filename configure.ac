#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Copyright (C) 2007, 2008, 2009  Sylvain Beucler

# This file is part of GNU FreeDink

# GNU FreeDink is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# GNU FreeDink is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
 
AC_PREREQ(2.61)
AC_INIT([MiniFE],[0.1],[bug-freedink@gnu.org])
AC_CONFIG_AUX_DIR(autotools)
# For future version of aclocal:
AC_CONFIG_MACRO_DIR(autotools/macros)
AM_INIT_AUTOMAKE([gnits dist-bzip2 dist-zip])

AC_CONFIG_HEADERS(config.h)
# Define $host $host_os $build
AC_CANONICAL_HOST
# Checks for programs.
AC_PROG_CXX


# Gnulib - configuration in gnulib/m4/gnulib-cache.m4
#gl_EARLY


# Explicitely call PKG_PROG_PKG_CONFIG because PKG_CHECK_MODULES is
# called conditionaly
PKG_PROG_PKG_CONFIG()

case "${host}" in
# (cf. (autoconf.info)Using System Type) (2002)
*-*-msdos* | *-*-go32* | *-*-mingw32* | *-*-cygwin* | *-*-windows* \
| mipsallegrexel-psp-elf)
    # do something
    ;;
*)
    # do something else
    ;;
esac
AM_CONDITIONAL(PSP, test "$host" == "mipsallegrexel-psp-elf")

# Lengthy Gnulib checks
#gl_INIT


dnl Check for SDL
SDL_VERSION=1.2.0
AC_ARG_VAR([SDL_CONFIG], [Path to the sdl-config utility, it needs to
  be redefined (and saved by autoconf) for cross-compilation.])
AM_PATH_SDL($SDL_VERSION,
  :,
  AC_MSG_WARN([*** SDL version $SDL_VERSION not found!]); LACK_LIBSDL=1
)
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
LIBS="$SDL_LIBS $LIBS"

LIBS="-ljpeg -lpng -lz $LIBS"
AC_CHECK_LIB(SDL_gfx,zoomSurface,,AC_MSG_WARN([You need to install SDL_gfx]); LACK_LIBSDL_GFX=1)
AC_CHECK_LIB(SDL_image,IMG_ReadXPMFromArray,,AC_MSG_WARN([You need to install SDL_image]); LACK_LIBSDL_IMAGE=1)
AC_CHECK_LIB(guichan,gcnGuichanVersion,,AC_MSG_WARN([You need to install Guichan >= 0.8]); LACK_LIBGUICHAN=1)
if test "$LACK_LIBGUICHAN" != "1"; then
   # In the end, we need -lguichan -lguichan_sdl -lguichan, to address
   # circular references
   LIBS="-lguichan -lguichan_sdl $LIBS"
fi


echo
echo "Errors and warnings recap:"
error=0
warning=0

if test "$LACK_LIBSDL" = 1; then
  echo "* You need to install the libSDL";
  error=1
fi
if test "$LACK_LIBSDL_GFX" = 1; then
  echo "* You need to install the SDL_gfx library";
  error=1
fi
if test "$LACK_LIBSDL_IMAGE" = 1; then
  echo "* You need to install the SDL_image library";
  error=1
fi

if test "$LACK_LIBGUICHAN" = 1; then
  echo "* You need to install the Guichan library (preferably v0.8.1)";
  error=1
fi

if test "$error" = 1; then
  echo
  echo "There are errors - stopping. Please check the reports above."
  echo "If dependencies (such as SDL_image) are missing,"
  echo "please check BUILD for distro package names."
  exit 1
elif test "$warning" = 1; then
  echo
  echo "There were warnings - let's continue for now,"
  echo "but please check make sure nothing is missing."
else
  echo "* No errors"
fi
echo


#AC_CONFIG_FILES([Makefile gnulib/lib/Makefile gnulib/po/Makefile.in src/Makefile])
AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT

echo
#echo "Options summary:"
#echo "* --enable-upx=$enable_upx"
#echo "* --enable-static=$enable_static"
#echo "* --enable-embedded-resources=$enable_embedded_resources"
echo
echo "Now you can type 'make' to compile $PACKAGE_STRING"
