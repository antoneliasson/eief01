#include <avr/io.h>
#include "pinfunctions.h"

void toggle_status_led(void)
{
    PORTC ^= _BV(PC5);
}

void set_error_led(void)
{
    PORTC |= 1<<PC4;
}

void toggle_status2(void)
{
    PORTC ^= _BV(PC4);
}

void toggle_status3(void)
{
    PORTC ^= _BV(PC3);
}

void toggle_heartbeat_led(void)
{
    PORTC ^= _BV(PC2);
}

void enable_poweramp(void)
{
    PORTD &= ~_BV(PD7);
}

void disable_poweramp(void)
{
    PORTD |= _BV(PD7);
}
