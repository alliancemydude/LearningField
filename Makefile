# Compiler and flags
CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra \
         -I. \
         -Icore \
         -Isim \
         -Ievo
LDFLAGS = -lm

# Find all .c files in current directory and subdirectories
SRC = $(wildcard *.c) $(wildcard */*.c)
OBJ = $(SRC:.c=.o)

TARGET = battle

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean