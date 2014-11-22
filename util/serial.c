/*
 * Acknowledgements:
 * - http://www.downtowndougbrown.com/2013/01/microcontrollers-interrupt-safe-ring-buffers/
 * - http://www.eit.lth.se/fileadmin/eit/courses/edi021/avr-libc-user-manual/group__avr__stdio.html
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"
#include "pinfunctions.h"

static volatile uint8_t ring_head;
static volatile uint8_t ring_tail;
static volatile unsigned char ring_buffer[SERIAL_RING_SIZE];

static int enqueue(unsigned char c, FILE *stream);

static FILE mystdout = FDEV_SETUP_STREAM(enqueue, NULL, _FDEV_SETUP_WRITE);

static void enable_transmission(void)
{
    UCSR0B |= 1<<UDRIE0;
}

static void disable_transmission(void)
{
    UCSR0B &= ~(1<<UDRIE0);
}

void serial_init(unsigned int ubrr)
{
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    // Enable interrupt on receive
    // Enable receiver and transmitter
    UCSR0B = 1<<RXCIE0 | 1<<RXEN0 | 1<<TXEN0;
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = 3<<UCSZ00;

    stdout = &mystdout;
}

static int enqueue(unsigned char c, FILE *stream)
{
    toggle_status2();
    if (c == '\n') {
        // Windows catering
        enqueue('\r', stream);
    }
    uint8_t next_head = (ring_head + 1) % SERIAL_RING_SIZE;
    if (next_head == ring_tail) {
        // buffer overflow
        return -1;
    } else {
        ring_buffer[ring_head] = c;
        ring_head = next_head;
        enable_transmission();
        return 0;
    }
}

static int dequeue(void)
{
    toggle_status3();
    if (ring_head == ring_tail) {
        return -1;
    } else {
        unsigned char c = ring_buffer[ring_tail];
        ring_tail = (ring_tail + 1) % SERIAL_RING_SIZE;
        return c;
    }
}

ISR(USART_UDRE_vect)
{
    toggle_status_led();
    int c = dequeue();
    if (c == -1) {
        // empty buffer
        disable_transmission();
    } else {
        UDR0 = c;
    }
}
