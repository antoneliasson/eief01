#include <stdio.h>
#include "serialport.h"

void print_welcome()
{
    printf("Getting values:\n"
           "> g variable\n"
           "Setting values:\n"
           "> s variable value\n"
           "Readable variables:\n"
           "    k (proportional gain), r (reference), t (integration time),\n"
           "    u (controller output ranging 0-255), y (current speed in RPM)\n"
           "Writable variables:\n"
           "    k, r, t\n"
           "Exit with EOF (C-d)\n"
           "> ");
}

int main()
{
    int fd = serial_init("/dev/ttyS0");
    FILE *serial = fdopen(fd, "r+");

    print_welcome();

    char buff[32];
    char *result;

    result = fgets(buff, sizeof(buff), stdin);
    while (result == buff) {
        fputs(buff, serial);
        result = fgets(buff, sizeof(buff), serial);
        if (result == buff) {
            fputs(buff, stdout);
        }
        fputs("> ", stdout);
        result = fgets(buff, sizeof(buff), stdin);
    }
    /* fputs("hej\n", serial); */
    /* printf("%d %s", result==buff, buff); */

    fclose(serial);
    serial_cleanup(fd);
    return 0;
}
