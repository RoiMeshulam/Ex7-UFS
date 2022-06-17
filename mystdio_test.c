#include "mystdio.h"

int main() {

    mymkfs(20000);
    mymount("myFileTest", "/root", NULL, NULL, NULL);

    myFILE *file1 = myfopen("/root/file1", "a");
    myFILE *file2 = myfopen("/root/file2", "w"); // test without dir before file
    myFILE *file3 = myfopen("/root/dir1/file3", "w"); // test without dir before file
    myFILE *file4 = myfopen("/root/dir2/file4", "r+");

    if (file1->fd != file2->fd) {
        printf("Successful check!\n");
    } else {
        printf("Bad case: Files fds should be different \n");
    }

    if (file3->fd != file4->fd) {
        printf("Successful check!\n");
    } else {
        printf("Bad case: Files fds should be different \n");
    }

    if (file1->fd != file4->fd) {
        printf("Successful check!\n");
    } else {
        printf("Bad case: Files fds should be different \n");
    }

    myFILE *file5 = myfopen("/root/dir3/file5", "r+"); //should create a new file
    myFILE *file6 = myfopen("/root/dir3/file6", "r"); // should be -1
    myFILE *file7 = myfopen("/root/dir3/file5", "r"); // should be OK

    if (file5->fd != -1 && file6->fd == -1) {
        printf("Successful check!\n");
    } else {
        printf("Bad case: Files fds should be different \n");
    }

    char buf[50] = "write to file2";
    char ans[50];
    int file2_write = myfwrite(buf, 14, 1, file5);
    int file2_read = myfread(ans, 15, 1, file7);

    if (!strcmp(ans, buf)) {
        printf("Successful check!\n");
    } else {
        printf("Bad case: The strings should be the same \n");
    }

    myfprintf(file4, "this is a number: %d,this is a char :%c, this is a float %f", 5, 'z', 5.8);
    char ans1[100];
    myread(file4->fd,ans1, get_size_inodes(file4->fd));
    int num;
    char c;
    double f;
    myfscanf(file4, "this is a number: %d,this is a char :%c, this is a float %f", &num,&c, &f);
    if(num == 5 && c =='z'){
        printf("Successful check!\n");
    }
    else{
        printf("Test myopen failed!\n");
    }

    myfclose(file1);
    myfclose(file2);
    myfclose(file3);
    myfclose(file4);
    myfclose(file5);
    myfclose(file6);
    myfclose(file7);

    printf("myFILE_test finished!\n");

    return 0;
}

