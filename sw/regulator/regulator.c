#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "regulator.h"
#include "pinfunctions.h"
#include "serial.h"

#define BAUDRATE 19200

// Formula from the ATmega88 datasheet, section 17.3.1. Without calibration, it
// appears to work only for BAUDRATE <= 9600:
#define UBRR F_CPU / BAUDRATE / 16 - 1

// This works for BAUDRATE <= 19200. I think my ATmega's internal clock needs
// calibration.
//#define UBRR F_CPU / BAUDRATE / 16

static volatile unsigned char AB_old;
static volatile int8_t counter;

static void inc_counter(void);
static void dec_counter(void);
static void set_duty_cycle(unsigned char);

static void init_pwm(void)
{
    // Power amplifier is initally disabled
    disable_poweramp();

    // Set OC0A and AMP_EN to output
    DDRD |= 1<<PD7 | 1<<PD6;

    // Timer 0 prescaler = f_clk_io / 8
    // => f_OC0A_PWM = 8000000 / 8 / 256 ~= 4 kHz
    TCCR0B = 1<<CS01;

    // Fast PWM, TOP=0xFF. Set OC0A on compare match, clear at BOTTOM
    TCCR0A = 1<<COM0A1 | 1<<COM0A0 | 1<<WGM01 | 1<<WGM00;
}

static void init_clock(void)
{
    // CTC. Timer 1 prescaler = f_clk_io / 64
    TCCR1B = 1<<WGM12 | 1<<CS11 | 1<<CS10;

    // Compare match at 10 Hz
    OCR1A = F_CPU / 64 / 10;

    // Interrupt on compare match
    TIMSK1 = 1<<OCIE1A;
}

int main(void)
{
    DDRC = 1<<5 | 1<<4 | 1<<3 | 1<<2;
    PORTB = 1<<2 | 1<<1; // enable pullups

    // See ../osccal program. The default is 155 for this chip.
    OSCCAL = 148;

    serial_init(UBRR);
    init_pwm();
    set_duty_cycle(0);
    init_clock();

    enable_poweramp();
    sei();

    printf("init done\n\n");

    while (1) {
        // René Sommer's algorithm
        unsigned char BA_new = PINB>>1 & 3;
        unsigned char sum = AB_old ^ BA_new;

        if (sum == 1) {
            inc_counter();
            set_duty_cycle(counter);
        } else if (sum == 2) {
            dec_counter();
            set_duty_cycle(counter);
        }
        // swap bits A and B
        AB_old = (BA_new>>1 & 1) | (BA_new<<1 & 2);
    }
    return 0;
}

static void inc_counter(void)
{
    if (counter < 127) {
        counter += 1;
        printf("incr to %d. \n", counter);
    }
}

static void dec_counter(void)
{
    if (counter > -127) {
        counter -= 1;
        printf("decr to %d. \n", counter);
    }
}

static void set_duty_cycle(unsigned char val)
{
    OCR0A = 255 - val;
}

ISR(USART_RX_vect)
{
    toggle_status_led();
    char received = UDR0;
    putchar(received); // echo it back
}

ISR(TIMER1_COMPA_vect)
{
    /* int16_t speed = 10*counter; */
    /* if (speed != 0) { */
    /*     printf("Speed = %d ticks per second\n", speed); */
    /* } */
    /* counter = 0; */
}
