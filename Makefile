CFLAGS=-Wall -Wextra -Wswitch-enum -std=c11 -pedantic
LIBS=

ibvm: main.c
	$(CC) $(CFLAGS) -o ibvm main.c $(LIBS)