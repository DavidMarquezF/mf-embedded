CC=avr-gcc
CFLAGS = --std=c99 -Os -Wall -mmcu=attiny85 \
	-fshort-enums -DF_CPU=1000000
LDFLAGS = -Os -mmcu=attiny85 -Wall

.PHONY: build-% clean veryclean

libmf.a: libmf.a(usi_twi_slave.o debug_utils.o)

%.hex: %
	avr-objcopy -Oihex $< $@

build-%: %.hex
	avrdude -c arduino -p atmega328p -P $(DEVICE) -U $<

clean:
	\rm -f *.o *.hex *~ 

veryclean: clean
	\rm -f *.a

usi_twi_slave.o: usi_twi_slave.h
debug_utils.o: debug_utils.h