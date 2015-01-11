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

// The rate at which the regulation interrupt is fired
#define REG_FREQ 2

// The number of ticks per revolution
#define TICKS_PER_REV 100

static volatile unsigned char AB_old;
static volatile unsigned char counter;

static volatile uint8_t u;
static volatile int16_t y;
static volatile double I;

static uint8_t yref = 0;
static const int umax = 255;
static const int umin = 0;

static unsigned int K = 2;
static unsigned int Ti = 1;
static unsigned int Tr = 1;
static const unsigned int beta = 1;

static void inc_counter(void);
static void dec_counter(void);
static void set_duty_cycle(unsigned char);

static void init_timers(void)
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

    // Interrupt on compare match
    TIMSK0 = 1<<OCIE0A;

    // CTC. Timer 1 prescaler = f_clk_io / 64
    TCCR1B = 1<<WGM12 | 1<<CS11 | 1<<CS10;

    // Compare match at REG_FREQ Hz
    OCR1A = F_CPU / 64 / REG_FREQ;

    // Interrupt on compare match
    TIMSK1 = 1<<OCIE1A;
}

static void process_cmds(void)
{
    char line[16];
    char *status = fgets(line, 16, stdin);
    if (status == 0) {
        printf("Error reading stdin.\n");
        set_error_led();
        return;
    }
    char cmd;
    char var;
    int val;
    int args = sscanf(line, "%c %c %d", &cmd, &var, &val);

    switch (cmd) {
    case 'g':
        if (args == 2) {
            switch (var) {
            case 'i':
                printf("%lf\n", I);
                break;
            case 'k':
                printf("%d\n", K);
                break;
            case 'r':
                printf("%d\n", yref);
                break;
            case 't':
                printf("%d\n", Ti);
                break;
            case 'u':
                printf("%d\n", u);
                break;
            case 'y':
                printf("%d\n", y); // rpm
                break;
            default:
                printf("Syntax error. Invalid variable %c.\n", var);
                break;
            }
        } else {
            printf("Syntax error. Expected 2 arguments, got %d\n", args);
        }
        break;
    case 's':
        if (args == 3) {
            switch (var) {
            case 'k':
                printf("OK\n");
                K = val;
                break;
            case 'r':
                printf("OK\n");
                yref = val;
                break;
            case 't':
                printf("OK\n");
                Ti = val;
                break;
            default:
                printf("Syntax error. Invalid variable %c.\n", var);
                break;
            }
        } else {
            printf("Syntax error. Expected 3 arguments, got %d\n", args);
        }
        break;
    default:
        printf("Syntax error. Invalid command %c\n", cmd);
        break;
    }
}

int main(void)
{
    DDRC = 1<<5 | 1<<4 | 1<<3 | 1<<2;
    PORTB = 1<<2 | 1<<1; // enable pullups

    // See ../osccal program. The default is 155 for this chip.
    OSCCAL = 148;

    serial_init(UBRR, 0);
    init_timers();
    set_duty_cycle(0);

    enable_poweramp();
    sei();

    printf("init done\n\n");

    while (1) {
        process_cmds();
    }
    return 0;
}

static void inc_counter(void)
{
    if (counter < 255) {
        counter += 1;
    }
}

static void dec_counter(void)
{
    if (counter > 0) {
        counter -= 1;
    }
}

static void set_duty_cycle(unsigned char val)
{
    OCR0A = 255 - val;
}

static int sat(int val, int max, int min)
{
    if (val > max) {
        return max;
    }
    if (val < min) {
        return min;
    }
    return val;
}

ISR(TIMER0_COMPA_vect)
{
    // René Sommer's algorithm
    unsigned char BA_new = PINB>>1 & 3;
    unsigned char sum = AB_old ^ BA_new;

    if (sum == 1) {
        inc_counter();
    } else if (sum == 2) {
        dec_counter();
    }
    // swap bits A and B
    AB_old = (BA_new>>1 & 1) | (BA_new<<1 & 2);
}

ISR(TIMER1_COMPA_vect)
{
    y = REG_FREQ * counter * 60 / TICKS_PER_REV;
    // Calculate output
    int e = yref - y;
    int v = K * (beta * yref - y) + I;
    u = sat(v, umax, umin);
    set_duty_cycle(u);
    // Update states
    I = I + (K / (REG_FREQ*(double)Ti)) * e + (u - v) / (REG_FREQ*(double)Tr);
    //yold = y;
    //printf("y = %d rpm. u = %d\n", y, u);
    counter = 0;
}
