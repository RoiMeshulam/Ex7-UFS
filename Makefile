CC=gcc
AR=ar
FLAGS= -Wall -g

all: test

test: test.o mymkfs.o myFile.o
	$(CC) $(FLAGS) -o test test.o mymkfs.o myFile.o

test.o: test.c
	$(CC) $(FLAGS) -c test.c

mymkfs.o: mymkfs.c mymkfs.h
	$(CC) $(FLAGS) -c mymkfs.c

myFile.o: myFILE.c mymkfs.h myFILE.h
	$(CC) $(FLAGS) -c myFILE.c

.PHONY: clean all
clean:
	rm -f *.o test