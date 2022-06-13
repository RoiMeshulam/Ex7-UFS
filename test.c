#include <stdio.h>
//#include "mymkfs.h"
#include "myFILE.h"



int main(){

    mymkfs(10000);
    mymount();
//    print_myfs();
//    myFILE *file = myfopen("/root/roi/first","r+");
//    if(file==NULL){
//        printf("ERRORRRRR\n");
//
//    }
//    print_myfs();
//    printf("%d\n",file->fd);
//    char buf[50]="fprintf check";
//    char read[50];
//    int y = myfread(read,12,1,file);
//    int x =myfwrite(buf,12,1,file);
//    char a= 'A';
//    int x = myfprintf(file,"%c) test %d of myfprintf2", a , 3);
//    printf("%d\n",x);
//    printf("%s\n",read);
//    myfclose(file);
//    print_myfs();
//    struct mydirent *sd;
//    myDIR *dir = myopendir("/root/roi");
//    printf("%d\n",dir->dirp); // should be 1
//    if (dir ==NULL){
//        printf("dir is null\n");
//        return 0;
//    }
//    while( (sd = myreaddir(dir)) != NULL   ){
////        printf("loop\n");
////        printf("%p\n",sd);
//        printf(">> %s\n",sd->name);
//    }
////    printf("after loop\n");
//    myclosedir(dir->dirp);


//    print_myfs();

    myDIR *dir1;
    myDIR *dir2;
//
    dir1= myopendir("/root/roi");
    dir2= myopendir("/root/naor");
    print_myfs();
    printf("fd1: %d\n",dir1->dirp);
    printf("fd2: %d\n",dir2->dirp);

    int myfd = myopen("/root/roi/third",0); //create
    int myfd1 = myopen("/root/roi/second",3); // write
    int myfd2 = myopen("/root/roi/second",2); // read
    int myfd3 = myopen("/root/roi/five",4); //r+w return -1
    int myfd4 = myopen("/root/roi/second",4); // r+w
//////
//////    // add test of differnt files have different fds;
//////
    printf("myfd : %d\n",myfd);
    printf("myfd : %d\n",myfd1);
    printf("myfd : %d\n",myfd2);
    printf("myfd : %d\n",myfd3); // should be -1 cannot create file wite flag 4
    printf("myfd : %d\n",myfd4);
//////////
    char buf[50]="check write and read";
    char ans[50];
    int w0= mywrite(myfd,buf,20); // -1 cannot write with flage 0;
    int w1 = mywrite(myfd1,buf,20); // create new file and write to blocks
    int r2 = myread(myfd2,ans,21);
    printf("w0 should be -1 :: %d\n",w0);
    ans[w1]='\0';
    printf("i write in w1 %d bytes\n",w1);
    printf("i read in w1 the string %s \n",ans);
    printf("read : %d \n",r2);

    myclose(myfd);
    myclose(myfd1);
    myclose(myfd2);
    myclose(myfd3);
//
////    char buf1[50]="check again the write function";
////    char ans1[50];
////    int w3= mywrite(myfd4,buf1,30);
////    int r3= myread(myfd1,ans1,30);
////    printf("i read in r3 the string %s \n",ans1);
////    int lseek = mylseek(myfd1,0,SEEK_SET);
////    int r4 = myread(myfd1,ans1,30);
////    printf("%d\n",r3==r4);
////
////
////    myclose(myfd4);
////
//    print_myfs();
//
//    mywrite(myfd,"appending to the end",20);
//    mywrite(myfd,"another check need to add to the previous string",48);
//    print_myfs();
//    mywrite(myfd," check2",7);
//
//    myread(myfd,buf,58);
//    printf("the string i read is %s \n",buf);
//    print_myfs();
//    myclose(myfd);
//    myclose(myfd1);
//    myclose(myfd2);
//    myclose(myfd3);
    print_myfs();
    printf("done\n");
    return 0;
}