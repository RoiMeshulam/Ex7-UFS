#ifndef EX7_UFS_MYMKFS_H
#define EX7_UFS_MYMKFS_H
#endif //EX7_UFS_MYMKFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_FILES 10000
#define DATASIZE 512

struct superblock{
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct inode{
    int first_block;
    int number_of_blocks;
    char name[8];
    int datasize;
    int isDir; // 1 Dir , 2 File
};

struct disk_block{
    int next_block_num;
    char data[DATASIZE];
};

struct myopenfile{
    int myfd; // the index of inode in inodes
    int flag; // Permissions
    int curr_seek //curr pos of the seek in the file ;
};

struct mydirent {
    int size;
    int fds[12];
    char name[12]; // Curr name
};

typedef struct myDIR{
    int dirp;
    int pos;
}myDIR;

//Ex functions
void mymkfs(size_t s); // initialize a new s-size file system
int mymount(); // load the file system
int myopen(const char *pathname,int flags);
int myclose(int myfd);
ssize_t myread(int myfd,char *buf,size_t count);
ssize_t mywrite(int myfd,const void *buf,size_t count);
off_t mylseek(int myfd,off_t offset, int whence);
myDIR* myopendir(const char *name);
struct mydirent *myreaddir(myDIR* dirp);
int myclosedir(int dirp);

//my function:
void print_myfs();
int add_to_myopenfile(int index,int flags);
int add_new_inode(const char* pathname);
void set_filesize(int myfd , int count);
int find_empty_block();
void shorten_file(int block_num);
int writeToBlocks(int firstBlock ,int pos, char* data , int count);
int readFromBlocks(int myfd,int pos, char *buf , size_t count);
int allocate_more_blocks(int myfd , int numOfBlocks);
void sync_fs();
void clean_data(int firstBlock);
