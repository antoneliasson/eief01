#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "helloserial.h"

#define BAUDRATE 4800
#define UBRR F_CPU / BAUDRATE / 16 - 1

static void usart_init(unsigned int ubrr)
{
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    // Enable interrupt on receive
    // Enable receiver and transmitter
    UCSR0B = 1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0;
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = 3<<UCSZ00;
}

int main(void)
{
    DDRC = 1<<5 | 1<<2;

    usart_init(UBRR);
    sei();

    while (1) {
        PORTC |= 1<<2;
        _delay_ms(500);
        PORTC &= ~(1<<2);
        _delay_ms(500);
    }
    return 0;
}

void toggle_status_led(void)
{
    if (PORTC & 1<<5) {
        PORTC &= ~(1<<5);
    } else {
        PORTC |= 1<<5;
    }
}

ISR(USART_RX_vect)
{
    toggle_status_led();
    char received = UDR0;
    UDR0 = received; // echo it back
}
