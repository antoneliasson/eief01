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

typedef uint8_t ring_pos_t;

struct circular_queue {
    ring_pos_t head; // first free position
    ring_pos_t tail; // first used position (to send)
    unsigned char buffer[SERIAL_RING_SIZE];
};

static volatile struct circular_queue tx_queue;
static volatile struct circular_queue rx_queue;

// Echo incoming characters immediately?
static volatile int echo;

static int enqueue_tx(unsigned char, FILE*);
static int dequeue_rx(FILE*);

static FILE mystdout = FDEV_SETUP_STREAM(enqueue_tx, NULL, _FDEV_SETUP_WRITE);
static FILE mystdin = FDEV_SETUP_STREAM(NULL, dequeue_rx, _FDEV_SETUP_READ);

static void enable_transmission(void)
{
    UCSR0B |= 1<<UDRIE0;
}

static void disable_transmission(void)
{
    UCSR0B &= ~(1<<UDRIE0);
}

void serial_init(unsigned int ubrr, int myecho)
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
    stdin = &mystdin;

    echo = myecho;
}

static int enqueue(unsigned char c, volatile struct circular_queue *queue)
{
    if (c == '\r') {
        // Carriage returns are evil. In Windows use PuTTY with settings
        // "Implicit CR in every LF" and "Implicit LF in every CR".
        c = '\n';
    }
    ring_pos_t next_head = (queue->head + 1) % SERIAL_RING_SIZE;
    if (next_head == queue->tail) {
        // buffer overflow
        set_error_led();
        return -1;
    } else {
        queue->buffer[queue->head] = c;
        queue->head = next_head;
        enable_transmission();
        return 0;
    }
}

static int enqueue_tx(unsigned char c, FILE *stream)
{
    return enqueue(c, &tx_queue);
}

static int enqueue_rx(unsigned char c) {
    return enqueue(c, &rx_queue);
}

static int queue_is_empty(volatile struct circular_queue *queue)
{
    return queue->head == queue->tail;
}

/*
 * Blocking queue polling function
 */
static int dequeue(volatile struct circular_queue *queue)
{
    while (queue_is_empty(queue))
        ;

    unsigned char c = queue->buffer[queue->tail];
    queue->tail = (queue->tail + 1) % SERIAL_RING_SIZE;
    return c;
}

static int dequeue_tx(void)
{
    return dequeue(&tx_queue);
}

static int dequeue_rx(FILE* stream)
{
    return dequeue(&rx_queue);
}

ISR(USART_UDRE_vect)
{
    toggle_status_led();
    if (queue_is_empty(&tx_queue)) {
        disable_transmission();
    } else {
        int c = dequeue_tx();
        UDR0 = c;
    }
}

ISR(USART_RX_vect)
{
    unsigned char received = UDR0;
    enqueue_rx(received);
    if (echo) {
        enqueue_tx(received, NULL);
    }
}
