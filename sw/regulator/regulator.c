#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "regulator.h"
#include "pinfunctions.h"
#include "serial.h"

#define BAUDRATE 9600 // doesn't transmit reliably faster than this
#define UBRR F_CPU / BAUDRATE / 16 - 1

static volatile unsigned char AB_old;
static volatile unsigned char counter;

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
    DDRC = 1<<5 | 1<<4 | 1<<3 | 1<<2;
    PORTB = 1<<2 | 1<<1; // enable pullups

    serial_init(UBRR);
    init_pwm();
    init_pci();

    enable_poweramp();
    sei();

    printf("init done\n\n");

    while (1) {
        // René Sommer's algorithm
        unsigned char BA_new = PINB>>1 & 3;
        unsigned char sum = AB_old ^ BA_new;

        //    if (sum == 1 || sum == 2) {
        if (sum == 1) {
            inc_counter();
            set_duty_cycle(counter);
        } else if (sum == 2) {
            dec_counter();
            set_duty_cycle(counter);
        }
        // swap bits A and B
        AB_old = (BA_new>>1 & 1) | (BA_new<<1 & 2);

        //    }

    }
    return 0;
}

static void inc_counter(void)
{
    if (counter < 255) {
        counter += 1;
        printf("incr to %d. \n", counter);
    }
}

static void dec_counter(void)
{
    if (counter > 0) {
        counter -= 1;
        printf("decr to %d. \n", counter);
    }
}

static void set_duty_cycle(unsigned char val)
{
    OCR0A = val;
}

ISR(USART_RX_vect)
{
    toggle_status_led();
    char received = UDR0;
    putchar(received); // echo it back
}

ISR(PCINT0_vect)
{
    /* // René Sommer's algorithm */
    /* unsigned char BA_new = PINB>>1 & 3; */
    /* unsigned char sum = AB_old ^ BA_new; */
    /* printf("PCINT, sum=%d. ", sum); */

    /* //    if (sum == 1 || sum == 2) { */
    /* if (sum == 1) { */
    /*     inc_counter(); */
    /*     set_duty_cycle(counter); */
    /* } else if (sum == 2) { */
    /*     dec_counter(); */
    /*     set_duty_cycle(counter); */
    /* } */
    /* // swap bits A and B */
    /* AB_old = (BA_new>>1 & 1) | (BA_new<<1 & 2); */

    /* printf("new AB: %d\n", AB_old); */
    /* toggle_status_led(); */
    /*     //    } */
}
