#include <stdio.h>
//#include "mymkfs.h"
#include "myFILE.h"



int main(){
    mymkfs(10000);
    mymount("another_one","/root",NULL,NULL,NULL);
    int myfd = myopen("/root/roi/first",0);
    int myfd1 = myopen("/root/roi/first",4);
    printf("myfd = %d\n",myfd);
    printf("myfd= %d\n",myfd1);

    char str[50]="write and read check in mod O_RDWR";
    char check_read[50];
    int write_read_check = mywrite(myfd1,str,34);
    int read_4 = myread(myfd1,check_read,35);

    printf("%d\n",write_read_check);
    printf("%d\n",read_4);
    printf("%s\n",str);
    printf("%s\n",check_read);



    print_myfs();
    printf("done\n");
    return 0;
}