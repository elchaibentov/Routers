# elchainentov
# Elcahi Ben tov 305500704
ex3: ex3.c
	$(CC) ex3.c -o ex3 $(CFLAGS)
	
clean:
	rm -f *.o ex3
# makefile for Ex3 - router

CC=gcc
CFLAGS=-Wall -Wvla -g -pthread

all: ex3
