bin_PROGRAMS = minife
minife_SOURCES = main.cpp
EXTRA_DIST=README background.png rpgfont.png klauncher/Makefile klauncher/klauncher.c

if PSP
TARGET=minife
PSP_EBOOT_TITLE = "GNU FreeDink - front-end"

all-local:
	ln -nfs $(srcdir)/klauncher .
	ln -nfs klauncher/klauncher.prx .
	$(MAKE) -C klauncher/
#	Based on pspdev/psp/sdk/lib/build.mak
#	Fixup is mandatory. Without it the homebrew will freeze, and
#	after 10-20s the PSP will shutdown abruptly.
	psp-fixup-imports $(TARGET) -o $(TARGET).elf
	mksfoex -d MEMSIZE=1 $(PSP_EBOOT_TITLE) PARAM.SFO
	psp-prxgen $(TARGET).elf $(TARGET).prx  # also strips
	pack-pbp EBOOT.PBP PARAM.SFO $(top_srcdir)/psp/freedink.png  \
		NULL $(top_srcdir)/psp/icon1.png $(top_srcdir)/psp/splash.png  \
		NULL  $(TARGET).prx NULL

clean-local:
	-$(MAKE) -C klauncher/ clean
	rm klauncher.prx

DEVICE=sdX1
install-psp:
	pmount /dev/$(DEVICE)
	cp -aL EBOOT.PBP klauncher.prx freedink.prx \
		$(srcdir)/rpgfont.png $(srcdir)/background.png \
		/media/$(DEVICE)/PSP/GAME/freedink/
	pumount /dev/$(DEVICE)
endif
