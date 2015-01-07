#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "osccal.h"
#include "serial.h"

#define BAUDRATE 19200

// Formula from the ATmega88 datasheet, section 17.3.1. Appears to work only for
// BAUDRATE <= 9600:
#define UBRR F_CPU / BAUDRATE / 16 - 1

// This works for BAUDRATE <= 19200. I think my ATmega's internal clock needs
// calibration.
//#define UBRR F_CPU / BAUDRATE / 16

int main(void)
{
    serial_init(UBRR);
    sei();

    printf("init done\n\n");
    printf("%d\n", OSCCAL);

    for (int i = 0; i <=255; ++i) {
        OSCCAL = i;
        _delay_ms(50);
        serial_init(UBRR);
        printf("OSCCAL=%d\n", OSCCAL);
        _delay_ms(50);
    }

    while (1) {
    }
    return 0;
}

ISR(USART_RX_vect)
{
    char received = UDR0;
    putchar(received); // echo it back
}
