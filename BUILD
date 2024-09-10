##
# Guichan
##

# Native compilation
apt-get install libguichan-dev

# PSP cross-compilation
# Time: < 5mn
VERSION=0.8.1
wget http://guichan.googlecode.com/files/guichan-$VERSION.tar.gz
tar xzf guichan-$VERSION.tar.gz
cd guichan-$VERSION.tar.gz
cp ../SDL_ttf/config.sub .
patch -p1 < configure.in.patch
aclocal   # for AM_PATH_SDL
autoconf  # for ./configure
mkdir cross-psp && cd cross-psp
# -lc at the end of LDFLAGS is mandatory, and it's also necessary to set it back in LIBS (g++-specific)
SDL_CONFIG=$(psp-config --psp-prefix)/bin/sdl-config \
  LDFLAGS="-L$(psp-config --pspsdk-path)/lib -lc" \
  LIBS="-lc -lpspuser -ljpeg -lpng -lz" \
  CXXFLAGS="-I$(psp-config --pspsdk-path)/include" \
  ../configure --host psp \
    --prefix=$(psp-config --psp-prefix) \
    --disable-allegro --disable-opengl --enable-sdl
make
make install


##
# MiniFE
##

# PSP cross-compilation
cp ../SDL_ttf/config.sub autotools/
mkdir cross-psp && cd cross-psp

# This line is inspired by the various README.PSP from pspdev
# (e.g. SDL_ttf's) plus a good deal of trial and error and
# $(SEARCH_ENGINE)ing
# (e.g. http://forums.ps2dev.org/viewtopic.php?p=65652). Don't ask me
# the details.  g++ generates more link errors than gcc, and the
# initial -lc (in LDFLAGS) is mandatory. The '-lstdc++ -lpsplibc'
# couple is needed to get rid of the infamous:
# 
# /mnt/h/pspdev/bin/../lib/gcc/psp/4.3.2/../../../../psp/lib/libc.a(setsockopt.o): In function `setsockopt':
# ../../../../../../newlib/libc/sys/psp/socket.c:293: undefined reference to `sceNetInetSetsockopt'
# ../../../../../../newlib/libc/sys/psp/socket.c:296: undefined reference to `sceNetInetGetErrno'
# /mnt/h/pspdev/bin/../lib/gcc/psp/4.3.2/../../../../psp/lib/libc.a(setsockopt.o): In function `getsockopt':
# ../../../../../../newlib/libc/sys/psp/socket.c:167: undefined reference to `sceNetInetGetsockopt'
# ../../../../../../newlib/libc/sys/psp/socket.c:170: undefined reference to `sceNetInetGetErrno'

SDL_CONFIG=$(psp-config --psp-prefix)/bin/sdl-config \
  LDFLAGS="-L$(psp-config --pspsdk-path)/lib -lc" \
  LIBS="-lc -lstdc++ -lpsplibc -lpspuser" \
  CXXFLAGS="-I$(psp-config --pspsdk-path)/include" \
  ../configure --host psp \
    --prefix=$(psp-config --psp-prefix) 

# Variant for PRX build:
SDL_CONFIG=$(psp-config --psp-prefix)/bin/sdl-config \
  LDFLAGS="-L$(psp-config --pspsdk-path)/lib -lc -specs=/usr/local/pspdev/psp/sdk/lib/prxspecs -Wl,-q,-T/usr/local/pspdev/psp/sdk/lib/linkfile.prx" \
  LIBS="-lc -lstdc++ -lpsplibc -lpspuser" \
  CPPFLAGS="-I$(psp-config --pspsdk-path)/include -DPSPFW3X -D_PSP_FW_VERSION=303" \
  CXXFLAGS="-I$(psp-config --pspsdk-path)/include" \
  ../configure --host psp \
    --prefix=$(psp-config --psp-prefix) 

# If you get: 'Error, found no relocation sections' when running
# psp-prxgen, you probably forgot to those compilation flags.

# For reference: minimal working g++ build:
#psp-g++ -I/usr/local/pspdev/psp/include/SDL -Dmain=SDL_main -I/usr/local/pspdev/psp/sdk/include -L/usr/local/pspdev/psp/sdk/lib -lc conftest.cpp -lc -lpspuser  


make

# Install:
# (replace PSP/GAME/freedink/EBOOT.PBP)
make install-psp DEVICE=sda1
