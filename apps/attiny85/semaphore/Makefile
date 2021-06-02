CC=avr-gcc
CFLAGS = --std=c99 -Os -Wall -mmcu=attiny85 \
	-fshort-enums -DF_CPU=1000000
LDFLAGS = -Os -mmcu=attiny85 -Wall

.PHONY: build clean

%.hex: %
	avr-objcopy -Oihex $< $@

build-%: %.hex
	avrdude -c usbtiny -p t85 -U $<

clean: 
	\rm -f *.o *.hex *~

main: main.o usi_twi_slave.o

main.o: usi_twi_slave.h
usi_twi_slave.o: usi_twi_slave.h