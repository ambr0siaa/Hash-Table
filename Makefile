CC = gcc
TAR = main
SRC = $(wildcard *.c)
CFLAGS = -Wall -Wextra -O2 -ggdb -g -flto

$(TAR) : $(SRC)
	$(CC) -o $(TAR) $(CFLAGS) $(SRC) 