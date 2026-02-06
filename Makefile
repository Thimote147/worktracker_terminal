# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = worktracker
SRC = worktracker.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) worktracker.dat

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
