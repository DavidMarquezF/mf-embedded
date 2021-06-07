//Based on https://github.com/rambo/TinyWire/blob/f9e0e5a5ffcb9b90691fb2214c7aa39e9cbcfd3e/TinyWireS
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <mflib/usi_twi_slave.h>
#include <util/delay.h>



#define _BV(n) (1 << n)
#define CHECK_BIT(var, pos) ((var) & (1 << (pos)))
#define SET_BIT(var, pos) ((var) |= (1<<(pos)))
#define UNSET_BIT(var, pos) ((var) &= (~(1<<(pos))))



/*
 * Set I2C Slave address. You can have multiple sensors with different addresses
 */
#define I2C_SLAVE_ADDRESS 4


#define LED1 PB4
#define BUTTON PB3
#define INTR PB1

volatile bool disableIntr = false;
ISR(PCINT0_vect)
{
  if(!CHECK_BIT(PINB, BUTTON)){
    //TOGGLE(LED1);
    disableIntr = true;

  }
  
}

volatile bool requestedInterrupt = false;
static void request_event(void)
{  
  //TOGGLE(LED1);
  if (requestedInterrupt){  
    usi_twi_transmit_byte(0xff);
    requestedInterrupt = false;
  }
         

  
}
static void receive_event(uint8_t data_length){
    if(data_length != 1)
    return;
    uint8_t val = usi_twi_receive_byte();

   if(val == 'i'){
     requestedInterrupt = true;

   }
  //PORTB |= 1<<LED1;
}



void main(void) {
  /*
   * Setup I2C
   */
  usi_twi_slave_init(I2C_SLAVE_ADDRESS);
  usi_twi_set_request_callback(request_event);
  usi_twi_set_receive_callback(receive_event);

  SET_BIT(PORTB, INTR);

  DDRB |= _BV(LED1) | _BV(INTR);
  DDRB &= ~_BV(BUTTON);

  UNSET_BIT(PORTB, LED1);

  PCMSK |= 1 << BUTTON;   // Active Interrupt on PB1
  GIMSK |= 1 << PCIE; // General Interrupt Mask Register / PCIE bit activates external interrupts
  MCUCR |= (1<<ISC01);    // Configuring as falling edge 
  
  sei();
  
  while(1){
    if(disableIntr){

     _delay_ms(25);
     if(!CHECK_BIT(PINB, BUTTON)){
        UNSET_BIT(PORTB, INTR);
        _delay_us(100);
        SET_BIT(PORTB, INTR);
     } 
      disableIntr = false;
     
    }
     
  }
}


