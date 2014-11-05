#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "dimmer.h"
#include "pinfunctions.h"

#define BAUDRATE 4800
#define UBRR F_CPU / BAUDRATE / 16 - 1

static void init_usart(unsigned int ubrr)
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

static void init_pwm(void)
{
    // Set OC0A to output
    DDRD |= 1<<PD6;

    // Timer 0 prescaler = f_clk_io / 8
    // => f_OC0A_PWM = 8000000 / 8 / 256 ~= 4 kHz
    TCCR0B = 1<<CS01;

    // Fast PWM, TOP=0xFF. Clear OC0A on compare match, set at BOTTOM
    TCCR0A = 1<<COM0A1 | 1<<WGM01 | 1<<WGM00;
}

int main(void)
{
    DDRC = 1<<5 | 1<<2;

    init_usart(UBRR);
    init_pwm();
    OCR0A = 0x7F; // 50 % duty cycle
    sei();

    while (1) {
        PORTC |= 1<<2;
        _delay_ms(500);
        PORTC &= ~(1<<2);
        _delay_ms(500);
    }
    return 0;
}

ISR(USART_RX_vect)
{
    toggle_status_led();
    char received = UDR0;
    UDR0 = received; // echo it back
}
