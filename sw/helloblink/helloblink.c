#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main()
{
    DDRC = 0b00111100;

    while (1) {
        PORTC |= 1<<5 | 1<<4 | 1<<3 | 1<<2;
        _delay_ms(500);
        PORTC &= ~(1<<5 | 1<<4 | 1<<3 | 1<<2);
        _delay_ms(500);
    }
}
