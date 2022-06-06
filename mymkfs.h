#ifndef EX7_UFS_MYMKFS_H
#define EX7_UFS_MYMKFS_H
#endif //EX7_UFS_MYMKFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 10000

struct superblock{
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct inode{
    int first_block;
    int size;
    char name[8];
};

struct disk_block{
    int next_block_num;
    char data[512];
};

struct myopenfile{
    int myfd; // the index of inode in inodes
    int flag; // Permissions


};

void mymkfs(size_t s); // initialize a new s-size file system
int mymount(); // load the file system
int myopen(const char *pathname,int flags);
int myclose(int myfd);
ssize_t myread(int myfd,void *buf,size_t count);
ssize_t mywrite(int myfd,const void *buf,size_t count);
//off_t mylseek(int myfd,off_t offset, int whence);
//myDIR *myopendir(const char *name);
//struct mydirent *myreaddir(myDIR *dirp);
//int myclosedir(myDIR *dirp);
void print_myfs();
int add_to_myopenfile(struct inode inode,int flags);
int add_new_inode(const char* pathname);