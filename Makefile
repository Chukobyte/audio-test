# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Ithirdparty/miniaudio

# Sources and target
SRC = src/audio_pthread.c src/main.c
TARGET = audio_test_app

# Default rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)
