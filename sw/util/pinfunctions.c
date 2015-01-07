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

void set_status2(void)
{
    PORTC |= 1<<PC4;
}

void toggle_status2(void)
{
    if (PORTC & 1<<PC4) {
        PORTC &= ~(1<<PC4);
    } else {
        PORTC |= 1<<PC4;
    }
}

void toggle_status3(void)
{
    if (PORTC & 1<<PC3) {
        PORTC &= ~(1<<PC3);
    } else {
        PORTC |= 1<<PC3;
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

void enable_poweramp(void)
{
    PORTD &= ~(1<<PD7);
}

void disable_poweramp(void)
{
    PORTD |= 1<<PD7;
}
