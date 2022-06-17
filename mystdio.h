#ifndef EX7_UFS_MYFILE_H
#define EX7_UFS_MYFILE_H
#endif //EX7_UFS_MYFILE_H

#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>


typedef struct myFILE {
    int fd;
}myFILE;

myFILE* myfopen(const char *restrict pathname, const char *mode);
int myfclose(myFILE *stream);
ssize_t myfread(void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream);
ssize_t myfwrite(const void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream);
int myfseek(myFILE *stream, long offset, int whence);
int myfscanf(myFILE * stream, const char * format, ...);
int myfprintf(myFILE * stream, const char * format, ...);
