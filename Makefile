CC = gcc
CFLAGS = -Wall -g
DIRS = bin
TARGET = simpledb
TARGET_DIR = bin

all: $(TARGET)

$(TARGET): main.c interface.o processor.o internals.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)/$(TARGET) main.c $(TARGET_DIR)/interface.o $(TARGET_DIR)/processor.o $(TARGET_DIR)/internals.o

interface.o: interface.c
	$(CC) $(CFLAGS) -c interface.c -o $(TARGET_DIR)/$@

processor.o: processor.c
	$(CC) $(CFLAGS) -c processor.c -o $(TARGET_DIR)/$@

internals.o: internals.c
	$(CC) $(CFLAGS) -c internals.c -o $(TARGET_DIR)/$@

clean:
	$(RM) -rd $(TARGET_DIR)

$(shell mkdir -p $(DIRS))