//Based on https://github.com/rambo/TinyWire/blob/f9e0e5a5ffcb9b90691fb2214c7aa39e9cbcfd3e/TinyWireS
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <mflib/usi_twi_slave.h>

/*
 * Set I2C Slave address. You can have multiple sensors with different addresses
 */
#define I2C_SLAVE_ADDRESS 3


#define LED1 PB4
#define LED2 PB3
#define LED3 PB1


static volatile uint8_t i2c_regs[] =
{
    0, 
};

static volatile uint8_t reg_position;
static const uint8_t reg_size = sizeof(i2c_regs);


/**
 * This function is executed when there is a request to read sensor
 * To get data, 2 reads of 8 bits are required
 * First requests send 8 older bits of 16bit unsigned int
 * Second request send 8 lower bytes
 * Measurement is executed when request for first batch of data is requested
 */
static void request_event(void)
{  

  usi_twi_transmit_byte(0);//i2c_regs[reg_position]);
  reg_position++;
  if (reg_position >= reg_size)
      reg_position = 0;
  
}
static void receive_event(uint8_t data_length){
    if(data_length != 1)
    return;
    uint8_t val = usi_twi_receive_byte();
    i2c_regs[0] = val;
    bool led1 = (val >> 0) & 1;
    bool led2 = (val >> 1) & 1;
    bool led3 = (val >> 2) & 1;

  
 
  //printByte(cmd);
  PORTB = led1 ? (PORTB | (1<<LED1)) : (PORTB & ~(1<<LED1));
  PORTB = led2 ? (PORTB | (1<<LED2)) : (PORTB & ~(1<<LED2));
  PORTB = led3 ? (PORTB | (1<<LED3)) : (PORTB & ~(1<<LED3));
  
  //PORTB |= 1<<LED1;
}

static void loop(void) {
  //set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  //sleep_enable();
  //sleep_mode();
  //sleep_disable(); 
  
}

void main(void) {
  /*
   * Setup I2C
   */
  usi_twi_slave_init(I2C_SLAVE_ADDRESS);
  usi_twi_set_request_callback(request_event);
  usi_twi_set_receive_callback(receive_event);
  
  sei();
  DDRB |= (1 << LED1) | (1<<LED2) | (1<<LED3);
  while(1)
      loop();
  
}


