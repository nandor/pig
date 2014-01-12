# Makefile for pig

CC=gcc
CFLAGS=-c -pedantic -Wall -Wextra -std=c99
LDFLAGS=-lc -lm -lpng
SOURCES=main.c pig.c vecmath.c rasterizer.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pig

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)