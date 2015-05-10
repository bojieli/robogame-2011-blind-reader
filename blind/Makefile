ALLOBJS=avr.hex scrmgrd reader menu snake bfb.ko startbdev daemon monitor
UAPPLIBS=libblindgui.h translate.h braille_string.h
FBDIR=$(PWD)/bfb
all:$(ALLOBJS)
daemon:daemon.c
	gcc -Wall $< -o $@ -lcurses
monitor:monitor.c
	gcc -Wall $< -o $@
startbdev:
	cp ./bfb/startbdev .
bfb.ko:
	$(MAKE) -C $(FBDIR)
	cp ./bfb/bfb.ko .
avr.hex:avr.c
	avr-gcc -mmcu=atmega128 -Wall $< -o tmp.elf
	avr-objcopy -j .text -j .data -O ihex tmp.elf $@
	rm tmp.elf
scrmgrd:scrmgrd.c transmission.h
	gcc $< -o $@
reader:reader.cpp $(UAPPLIBS)
	g++ $< -o $@ -lpthread -lboost_system
menu:menu.cpp $(UAPPLIBS)
	g++ $< -o $@ `pkg-config --cflags --libs libxml++-2.6` -lpthread -lboost_system
snake:snake.cpp $(UAPPLIBS)
	g++ $< -o $@ -lpthread -lboost_system
clean:
	rm -rf $(ALLOBJS)
	$(MAKE) -C $(FBDIR) clean
