CC = gcc
CFLAGS = -Wall -Wextra -g

ls : utils.o ls.o utils.h ls.h
	$(CC) $(CFLAGS) ls.o utils.o -o ls

utils.o: utils.c utils.h
	$(CC) $(CLFAGS) -c utils.c

ls.o: ls.c ls.h
	$(CC) $(CFLAGS) -c ls.c
clean:
	rm -f ls.o utils.o 
