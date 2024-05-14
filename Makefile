CC = gcc
TAR = main
SRC = $(wildcard *.c)
CFLAGS = -Wall -Wextra -O3 -flto

$(TAR) : $(SRC)
	$(CC) -o $(TAR) $(CFLAGS) $(SRC) 