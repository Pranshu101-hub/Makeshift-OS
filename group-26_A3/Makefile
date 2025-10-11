# Compiler and flags (warnings and debugging)
CC = gcc
CFLAGS = -Wall -Wextra -g

all: shell simplescheduler test_1 test_2

shell: shell.c
	$(CC) $(CFLAGS) -o shell shell.c

simplescheduler: simplescheduler.c
	$(CC) $(CFLAGS) -o simplescheduler simplescheduler.c

test_1: test_1.c dummy_main.h
	$(CC) $(CFLAGS) -o test_1 test_1.c

test_2: test_2.c dummy_main.h
	$(CC) $(CFLAGS) -o test_2 test_2.c

clean:
	rm -f shell simplescheduler test_1 test_2

