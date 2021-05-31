
#include <OneWire.h>
#include <DallasTemperature.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define ONEWIRE_BUSS PB4

OneWire oneWire(ONEWIRE_BUSS);
DallasTemperature sensors(&oneWire);

#define CS PB3   // Chip select
#define LED PB4  // Test LED
#define MISO PB1 // MISO or Data Out
#define MOSI PB0
#define SCK PB2 // Clock
#define INVALID_CMD 'z'

volatile byte command = 0;

volatile unsigned int pos;
volatile char cmd;

void turnOn()
{
  PORTB |= 1 << LED;
}
void turnOff()
{
  PORTB &= ~(1 << LED);
}

void turnOnOff(bool led)
{
  if (led)
    turnOn();
  else
    turnOff();
}

void printByte(uint8_t bt)
{
  turnOn();
  delay(4000);
  turnOff();
  delay(500);
  for (int i = 0; i < 8; i++)
  {
    turnOnOff((bt >> i) & 1);
    delay(1000);
  }
  turnOn();
  delay(4000);
  turnOff();
}

void printNumber(uint8_t number)
{
  for (int i = 0; i < number; i++)
  {
    turnOn();
    delay(1000);
    turnOff();
    delay(1000);
  }
}

double temp;
void setup(void)
{

  sensors.begin();
  cli();

  //MOSI:  PB0 input
  //MISO: PB1 output
  //SCK:  PB2 input
  DDRB |= (1 << LED) | (1 << MISO);    //0000 0010
  DDRB &= ~((1 << MOSI) | (1 << SCK)); //0000 0101

  USICR = (1 << USIWM0) | (1 << USICS1); // Three wire mode with extenral clock

  PORTB &= ~(0 << LED); // Turn PB1 off

  PCMSK |= 1 << CS;   // Active Interrupt on PB1
  GIMSK |= 1 << PCIE; // General Interrupt Mask Register / PCIE bit activates external interrupts

  sei();
} // end of setup
volatile bool isDown = false;
ISR(PCINT0_vect)
{

  if ((PINB & (1 << CS)) == 0)
  {
    if (!isDown)
    {
      isDown = true;
      // If edge is falling, the command and index variables shall be initialized
      // and the 4-bit overflow counter of the USI communication shall be activated:
      cmd = 0;
      USICR |= (1 << USIOIE); // Turn on receive interrupt
      USISR = 1 << USIOIF; // Clear Overflow bit
    }
  }
  else if (isDown)
  {
    isDown = false;
    // If edge is rising, turn the 4-bit overflow interrupt off:
    USICR &= ~(1 << USIOIE);
  }
}

// SPI interrupt routine, byte has overflowed
ISR(USI_OVF_vect)
{
  // If just initialized
  if(cmd == 0){
    cmd = USIDR;
    pos = 0;
  }

  // Prepare buffer
  switch (cmd)
  {
  case 'g':
  {
    const byte *buf = (const byte *)&temp;
    USIDR = buf[pos];
    if (++pos >= sizeof(double)) // Don't do sizeof(buf) since it is a pointer
      cmd = INVALID_CMD;
  }
  break;
  case 'i':
    USIDR = 2;
    break;
  case INVALID_CMD:
    USIDR = 0;
    break;
  default:

    // Default option of Switch-Case. Send 'cmd' back for debugging.
    USIDR = cmd;
    break;
  }
  // Clear counter overflow flag so next byte can begin arriving
  // While we're processing this one
  USISR = 1 << USIOIF;
} // end of interrupt service routine (ISR) USI_OVF_vect

void loop(void)
{

  if (sensors.getDS18Count() != 0) {
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
  }
  delay(1000);
} // end of loop
