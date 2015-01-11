#include <stdio.h>
#include "serialport.h"

int main()
{
    int fd = serial_init("/dev/ttyS0");
    FILE *serial = fdopen(fd, "r+");

    char line[16];
    char *result;

    fputs("hej\n", serial);

    result = fgets(line, 16, serial);
    printf("%d %s", result==line, line);

    fclose(serial);
    serial_cleanup(fd);
    return 0;
}
