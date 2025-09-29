# Makefile for sofia2hdf5 C implementation
# Copyright (C) 2025 Peter Kamphuis

# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -I.
LIBS = -lhdf5 -lm

# Source files
SOURCES = main.c common.c config.c parameter.c reader.c hdf5_writer.c utils.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = sofia2hdf5

# Build rules
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Dependencies (simplified - in practice, could use gcc -MM)
common.o: common.c common.h
config.o: config.c config.h common.h
parameter.o: parameter.c parameter.h common.h
reader.o: reader.c reader.h common.h parameter.h utils.h
hdf5_writer.o: hdf5_writer.c hdf5_writer.h common.h reader.h utils.h
utils.o: utils.c utils.h common.h parameter.h
main.o: main.c common.h config.h parameter.h reader.h hdf5_writer.h utils.h

# Installation (optional)
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

# Clean all generated files
distclean: clean
	rm -f *~

.PHONY: all clean distclean install