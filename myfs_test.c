#include <stdio.h>
#include "myfs.h"

int main(){
    mymkfs(10000);
    mymount("testDisc","/root",NULL,NULL,NULL);
    myDIR *dir1;
    myDIR *dir2;
    myDIR *dir3;
    myDIR *dir4;
    struct mydirent* firstdir;
    struct mydirent* seconddir;

    // making two different directories need to get different fds
    dir1= myopendir("/root/roi");
    dir2= myopendir("/root/naor");
    if (dir1->dirp != dir2->dirp){
        printf("Successful check\n");
    }else{
        printf("Bad case: dir_fd should be different \n");
    }

    //myOpen test
    int myfd = myopen("/root/roi/third",0); //create
    int myfd1 = myopen("/root/roi/second",3); // write
    int myfd2 = myopen("/root/roi/second",2); // read
    int myfd3 = myopen("/root/roi/five",4); //r+w return -1
    int myfd4 = myopen("/root/roi/four",0); // create
    int myfd5 = myopen("/root/roi/four",4); // r+w


    if (myfd != myfd1){
        printf("Successful check\n");
    }
    else{
        printf("Bad case: my_fd should be different \n");
    }

    if (myfd1 != myfd2){
        printf("Successful check\n");
    }
    else{
        printf("Bad case: my_fd should be different \n");
    }

    if (myfd3 != -1){
        printf("Successful check\n");
    }
    else{
        printf("Bad case: Cannot create file in read mode \n");
    }

    char buf[50]="Let's check mywrite and myread functions";
    char ans[50];
    char write_again[40]="I need to erase all the previous string";

    int failed_write = mywrite(myfd,buf,40); // should return -1
    if (failed_write == -1){
        printf("Successful check\n");
    }
    else{
        printf("Bad case: Cannot write in O_CREAT mode \n");
    }
    int w0= mywrite(myfd1,buf,40);
    int r0= myread(myfd2,ans,41);

    if (!strcmp(ans,buf)){
        printf("Successful check\n");
    } else{
        printf("Bad case: read failed \n");
    }
    memset(ans,'\0',50);
    int w1 = mywrite(myfd1,write_again,39); // create new file and write to blocks
    mylseek(myfd2,0,SEEK_SET);
    int r2 = myread(myfd2,ans,40);
    if (!strcmp(ans,write_again)){
        printf("Successful check\n");
    } else{
        printf("Bad case: read failed \n");
    }
    dir3= myopendir("/root/roi/shmuel");
    dir4= myopendir("/root/naor/arik");
    int myfd6 = myopen("/root/naor/dana",0);

    if (dir1 ==NULL){
        printf("Error! Unable to open directory.\n");
        return 0;
    }

    //should print this:
    //>> .
    //>> ..
    //>> third
    //>> second
    //>> four
    //>> shmuel
    printf("firstdir files and directories\n");
    while ( (firstdir = myreaddir(dir1)) != NULL ){
        printf(">> %s\n",firstdir->name);
    }

    if (dir2 ==NULL){
        printf("Error! Unable to open directory.\n");
    }
    //should print this:
    //>> .
    //>> ..
    //>> arik
    //>> dana

    printf("Second files and directories\n");
    while ( (seconddir = myreaddir(dir2)) != NULL ){
        printf(">> %s\n",seconddir->name);
    }

    char str[50]="write and read check in mod O_RDWR";
    char check_read[50];
    int write_read_check = mywrite(myfd5,str,34);
    int read_4 = myread(myfd5,check_read,35);
    if (!strcmp(str,check_read)){
        printf("Failed test! they should different strings \n");
    }else{
        printf("Successful check\n");
    }

    myclose(myfd);
    myclose(myfd1);
    myclose(myfd2);
    myclose(myfd3);
    myclose(myfd4);
    myclose(myfd5);
    myclose(myfd6);
    myclosedir(dir1->dirp);
    myclosedir(dir2->dirp);
    myclosedir(dir3->dirp);
    myclosedir(dir4->dirp);
    printf("mymkfs_test over\n");
    return 0;
}