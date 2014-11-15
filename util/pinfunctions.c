#include <avr/io.h>
#include "pinfunctions.h"

void toggle_status_led(void)
{
    if (PORTC & 1<<PC5) {
        PORTC &= ~(1<<PC5);
    } else {
        PORTC |= 1<<PC5;
    }
}

void toggle_heartbeat_led(void)
{
    if (PORTC & 1<<PC2) {
        PORTC &= ~(1<<PC2);
    } else {
        PORTC |= 1<<PC2;
    }
}
