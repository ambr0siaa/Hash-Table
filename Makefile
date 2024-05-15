CC = gcc
TAR = main
SRC = $(wildcard *.c)
CFLAGS = -Wall -Wextra -O2 -flto -ggdb

$(TAR) : $(SRC)
	$(CC) -o $(TAR) $(CFLAGS) $(SRC) 