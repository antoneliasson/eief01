#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <termios.h>

/*
 * Open serial port.
 * Returns file descriptor
 */
int serial_init(char *modemdevice);

/* 
 * restore the old port settings 
 */
void serial_cleanup(int fd);

#endif
