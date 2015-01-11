CSTANDARD = -std=gnu99
CFLAGS = -Wall -Wextra -Werror -pedantic
CFLAGS += $(CSTANDARD)

all:
	gcc $(CFLAGS) -o comm main.c serialport.o

libs:
	gcc $(CFLAGS) -c serialport.c
