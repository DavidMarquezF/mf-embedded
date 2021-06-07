CC=avr-gcc
CFLAGS = --std=c99 -Os -Wall -mmcu=attiny85 -fshort-enums \
	 -DF_CPU=1000000 -I../
LDFLAGS = -Os -mmcu=attiny85 -Wall

vpath lib% ../mflib

.PHONY: build-% clean veryclean

mflib:
	$(MAKE) -C ../mflib

%.hex: %
	avr-objcopy -Oihex $< $@

build-%: %.hex
	avrdude -c usbtiny -p t85 -U $<

clean: 
	\rm -f *.o *.hex *~
	$(MAKE) -C ../mflib clean

veryclean: clean
	\rm -f main
	$(MAKE) -C ../mflib veryclean

main: main.o -lmf

