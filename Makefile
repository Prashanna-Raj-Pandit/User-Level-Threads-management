CC = gcc
CFLAGS = -Wall -Wextra -g -Wno-unused-parameter -Wno-unused-variable
TARGET = uthreads

all: $(TARGET)

$(TARGET): uthreads.o
	$(CC) $(CFLAGS) -o $(TARGET) uthreads.o

uthreads.o: uthreads.c uthreads.h
	$(CC) $(CFLAGS) -c uthreads.c

clean:
	rm -f *.o $(TARGET)