CC=gcc
AR=ar
FLAGS= -Wall -g

all: libmyfs.so libmylibc.so

test: mystdio_test myfs_test

libmylibc.so: mystdio.o myfs.o
	$(CC) --shared -o libmylibc.so mystdio.o myfs.o

libmyfs.so: myfs.o
	$(CC) --shared -fpic -o libmyfs.so myfs.o

mystdio_test: mystdio.o mystdio_test.o myfs.o
	$(CC) $(FLAGS) -o mystdio_test mystdio_test.o myfs.o mystdio.o

myfs_test: myfs_test.o myfs.o
	$(CC) $(FLAGS) -o myfs_test myfs_test.o myfs.o

mystdio_test.o: mystdio_test.c
	$(CC) $(FLAGS) -c mystdio_test.c

myfs_test.o: myfs_test.c
	$(CC) $(FLAGS) -c myfs_test.c

myfs.o: myfs.c myfs.h
	$(CC) $(FLAGS) -c myfs.c

mystdio.o: mystdio.c myfs.h mystdio.h
	$(CC) $(FLAGS) -c mystdio.c

.PHONY: clean all
clean:
	rm -f *.o mystdio_test myfs_test libmyfs.so libmylibc.so