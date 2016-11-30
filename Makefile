CC = gcc
CFLAGS= -Wall -Werror -g

miniDB: movie.c miniDB.c
	$(CC) $(CFLAGS) -o miniDB miniDB.c movie.c 

clean:
	rm -f miniDB