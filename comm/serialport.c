#include "serialport.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>


/* baudrate settings are defined in <asm/termbits.h>, which is
 * included by <termios.h> */
#ifndef BAUDRATE
#define BAUDRATE B2400
#endif

#define _POSIX_SOURCE 1		/* POSIX compliant source */

static int fd, c, res;
static struct termios oldtio, newtio;
static char *device;

int serial_init(char *modemdevice)
{
    /* 
     * Open modem device for reading and writing and not as controlling tty
     * because we don't want to get killed if linenoise sends CTRL-C.
     **/
    device = modemdevice;
    //fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open (device, O_RDWR | O_NOCTTY );
    if (fd < 0)
      {
	  perror (device);
	  exit(-1);
      }

    tcgetattr (fd, &oldtio);	/* save current serial port settings */
    bzero (&newtio, sizeof (newtio));	/* clear struct for new port settings */

    /* 
     *BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     *CRTSCTS : output hardware flow control (only used if the cable has
     *all necessary lines. )
     *CS8     : 8n1 (8bit,no parity,1 stopbit)
     *CLOCAL  : local connection, no modem contol
     *CREAD   : enable receiving characters
     **/
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

    /*
     *IGNPAR  : ignore bytes with parity errors
     *ICRNL   : map CR to NL (otherwise a CR input on the other computer
     *          will not terminate input)
     *          otherwise make device raw (no other input processing)
     **/
    newtio.c_iflag = IGNPAR | ICRNL;

    /*
     * Map NL to CR NL in output.
     *                  */
#if 0
    newtio.c_oflag = ONLCR;
#else
    newtio.c_oflag = 0;
#endif
    

    /*
     * ICANON  : enable canonical input
     *           disable all echo functionality, and don't send signals to calling program
     **/
#if 1
    newtio.c_lflag = ICANON;
#else
    newtio.c_lflag = 0;
#endif

    /* 
     * initialize all control characters 
     * default values can be found in /usr/include/termios.h, and are given
     * in the comments, but we don't need them here
     *                                       */
    newtio.c_cc[VINTR] = 0;	/* Ctrl-c */
    newtio.c_cc[VQUIT] = 0;	/* Ctrl-\ */
    newtio.c_cc[VERASE] = 0;	/* del */
    newtio.c_cc[VKILL] = 0;	/* @ */
    newtio.c_cc[VEOF] = 4;	/* Ctrl-d */
    newtio.c_cc[VTIME] = 0;	/* inter-character timer unused */
    newtio.c_cc[VMIN] = 1;	/* blocking read until 1 character arrives */
    newtio.c_cc[VSWTC] = 0;	/* '\0' */
    newtio.c_cc[VSTART] = 0;	/* Ctrl-q */
    newtio.c_cc[VSTOP] = 0;	/* Ctrl-s */
    newtio.c_cc[VSUSP] = 0;	/* Ctrl-z */
    newtio.c_cc[VEOL] = 0;	/* '\0' */
    newtio.c_cc[VREPRINT] = 0;	/* Ctrl-r */
    newtio.c_cc[VDISCARD] = 0;	/* Ctrl-u */
    newtio.c_cc[VWERASE] = 0;	/* Ctrl-w */
    newtio.c_cc[VLNEXT] = 0;	/* Ctrl-v */
    newtio.c_cc[VEOL2] = 0;	/* '\0' */

    /* 
     * now clean the modem line and activate the settings for the port
     **/
    tcflush (fd, TCIFLUSH);
    tcsetattr (fd, TCSANOW, &newtio);

    /*
     * terminal settings done, return file descriptor
     **/
    
    return fd;
}

void serial_cleanup(int ifd){
    if(ifd != fd) {
	    fprintf(stderr, "WARNING! file descriptor != the one returned by serial_init()\n");
    }
    /* restore the old port settings */
    tcsetattr (ifd, TCSANOW, &oldtio);
}
