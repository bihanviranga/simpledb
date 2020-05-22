CC = gcc
CFLAGS = -Wall
DIRS = bin
TARGET = simpledb
TARGET_DIR = bin

all: $(TARGET)

$(TARGET): main.c interface.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)/$(TARGET) main.c $(TARGET_DIR)/interface.o

interface.o: interface.c
	$(CC) -c interface.c -o $(TARGET_DIR)/$@

clean:
	$(RM) -rd $(TARGET_DIR)

$(shell mkdir -p $(DIRS))