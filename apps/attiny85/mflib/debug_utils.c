#include <util/delay.h>
#include <avr/io.h>

#define TURN_ON(L) (PORTB |= 1 << (L))
#define TURN_OFF(L) (PORTB &= ~(1<<(L)))
#define TOGGLE(L) (PORTB ^= 1 << (L))

static uint8_t led1;
static uint8_t led2;


static void start_sequence(void){
    TURN_ON(led1);
    _delay_ms(500);
    TURN_OFF(led1);
    _delay_ms(500);
    TURN_ON(led1);
    _delay_ms(500);
    TURN_OFF(led1);
}

void debug_utils_print_byte(uint8_t byte){
    start_sequence();
    for (int i = 7; i >=0; i--)
    {
        TOGGLE(led2);
        if((byte >> i) & 1)
            TURN_ON(led1);
        else
            TURN_OFF(led1);

        _delay_ms(1000);
    }
    start_sequence();
    TURN_OFF(led2);
}


void debug_utils_init(uint8_t l1, uint8_t l2){
    led1 = l1;
    led2 = l2;
}
