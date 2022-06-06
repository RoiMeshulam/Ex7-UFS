#include <stdio.h>
#include "mymkfs.h"


int main(){

//    mymkfs(10000);
    mymount();
    print_myfs();
    myopen("first",1);
    print_myfs();
    printf("done\n");


    return 0;
}