CC      = g++
CFLAGS  = -Wall -Werror 

.PHONY: all clean

build:
	$(CC) $(CFLAGS) tema1.cpp -o tema1 -lpthread

all: tema1

clean:
	rm -f tema1