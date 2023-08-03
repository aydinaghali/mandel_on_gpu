CC = gcc
CPP = g++
CFLAGS = -g -Wall -O3 -I./metal-cpp
CPPFLAGS = $(CFLAGS) -std=c++17
LDFLAGS = -framework CoreGraphics -framework Metal -framework QuartzCore -framework Foundation
all: build run

bmp.o: bmp.c bmp.h
	$(CC) $(CFLAGS) -c bmp.c -o bmp.o

main.o: main.cpp bmp.h
	$(CPP) $(CPPFLAGS) -c main.cpp -o main.o

main: main.o bmp.o
	$(CPP) $(LDFLAGS) main.o bmp.o -o main

build: main

run:
	./main

clean:
	rm -rf main main.o bmp.o img.bmp

