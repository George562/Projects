CC=g++
CFLAGS=-lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -O3

all: main

main: main.o
	$(CC) main.o -o run $(CFLAGS)

main.o: main.cpp tools.h
	$(CC) main.cpp -c -O3