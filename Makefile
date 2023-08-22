OBJ := main.o
CC = gcc
CFLAGS = -c -Wall -std=gnu99
LDFLAGS = -lncurses
PROG = manlib

all: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(PROG)

main.o: ./main.c
	$(CC) $(CFLAGS) ./main.c

clean:
	rm $(PROG) *.o
