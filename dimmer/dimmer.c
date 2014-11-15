#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "dimmer.h"
#include "pinfunctions.h"

#define BAUDRATE 38400 // practical max with 8 MHz clock
#define UBRR F_CPU / BAUDRATE / 16 - 1

volatile unsigned char AB_old;
volatile unsigned char counter;

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

static void init_pci(void)
{
    // Enable pin change interrupt on PCINT7..0
    PCICR = 1<<PCIE0;
    // Enable pin change interrupt on PCINT1, PCINT0
    PCMSK0 = 1<<PCINT1 | 1<<PCINT0;
}

int main(void)
{
    DDRC = 1<<5 | 1<<2;
    PORTB = 1<<2 | 1<<1; // enable pullups

    init_usart(UBRR);
    init_pwm();
    init_pci();

    OCR0A = 0x40; // 25 % duty cycle
    sei();

    unsigned char old_counter = 0;
    while (1) {
        if (old_counter != counter) {
            UDR0 = counter;
            old_counter = counter;
        }
        toggle_heartbeat_led();
        _delay_ms(500);
    }
    return 0;
}

static void incCounter(void)
{
    if (counter < 255) {
        counter += 1;
    }
}

static void decCounter(void)
{
    if (counter > 0) {
        counter -= 1;
    }
}

ISR(USART_RX_vect)
{
    toggle_status_led();
    char received = UDR0;
    UDR0 = received; // echo it back
}

ISR(PCINT0_vect)
{
    // René Sommer's algorithm
    unsigned char BA_new = PINB>>1 & 3;
    unsigned char sum = AB_old ^ BA_new;
    if (sum == 1) {
        incCounter();
    } else if (sum == 2) {
        decCounter();
    }
    // swap bits A and B
    AB_old = (BA_new>>1 & 1) | (BA_new<<1 & 2);
}
