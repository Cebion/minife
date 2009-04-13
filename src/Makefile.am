bin_PROGRAMS = minife
minife_SOURCES = main.cpp
EXTRA_DIST=fixedfont.bmp

PSP_EBOOT_TITLE = minife
# pack-pbp <icon0.png> <icon1.pmf> <pic0.png> <pic1.png> <snd0.at3> <data.psp> <data.psar>
#PSP_EBOOT_ICON=freedink.png
##PSP_EBOOT_ICON1=
#PSP_EBOOT_UNKPNG=icon1.png
#PSP_EBOOT_PIC1=splash.png
##PSP_EBOOT_SND0
##PSP_EBOOT_PSAR

TARGET=minife
all-local:

#	Fixup is mandatory. Without it the homebrew will freeze, and
#	after 10-20s the PSP will shutdown abruptly.
	psp-fixup-imports $(TARGET) -o $(TARGET).elf
	mksfo $(PSP_EBOOT_TITLE) PARAM.SFO
#	psp-strip $(TARGET).elf -o $(TARGET)_strip.elf
	psp-prxgen $(TARGET).elf $(TARGET).prx  # also strips
	pack-pbp EBOOT.PBP PARAM.SFO NULL  \
		NULL NULL NULL  \
		NULL  $(TARGET).prx NULL
	rm -f $(TARGET)_strip.elf

DEVICE=sda1
install-psp:
	pmount /dev/$(DEVICE)
	mkdir -p /media/$(DEVICE)/PSP/GAME/$(TARGET)
	cp -a EBOOT.PBP $(srcdir)/fixedfont.bmp /media/$(DEVICE)/PSP/GAME/$(TARGET)/
	pumount /dev/$(DEVICE)
