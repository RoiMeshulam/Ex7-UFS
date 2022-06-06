#include "mymkfs.h"
//global structs
struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;
struct myopenfile *mof=NULL;

int add_new_inode(const char* pathname){
    printf("add new inode\n");
    for (int i = 0; i <sb.num_inodes ; ++i) {
        if (inodes[i].first_block == -1){
            strcpy(inodes[i].name,pathname); // check if path name has 8 byte only
            printf("inode name : %s\n",inodes[i].name);
            inodes[i].first_block=-2; // in use
            return i;
        }
    }
    return -1; // return -1 if there is no available inode
}

int add_to_myopenfile(struct inode inode,int flags){
    for (int i = 0; i <MAX_FILES ; ++i) {
        // the first empty place will be for the new open file
        if (mof[i].myfd == -1){
            mof[i].myfd= inode.first_block;
            mof[i].flag=flags;
            return mof[i].myfd;
        }
    }
    return -1;
}

// initialize a new s-size file system
void mymkfs(size_t s) {
    int x= s * 0.1; //10% of s
    sb.size_blocks= sizeof(struct disk_block);
    sb.num_inodes= x/ sizeof(struct inode);
    sb.num_blocks=(s-x- sizeof(struct superblock));
    sb.num_blocks = sb.num_blocks / sb.size_blocks;
    sb.size_blocks=sizeof (struct disk_block);
    printf("sb.num_inodes : %d\n",sb.num_inodes);
    printf("sb.num_blocks : %d\n",sb.num_blocks);
    printf("sb.size_blocks : %d\n",sb.size_blocks);

    inodes=malloc(sizeof(struct inode) * sb.num_inodes);
    for (int i = 0; i <sb.num_inodes ; ++i) {
        inodes[i].size = -1;
        inodes[i].first_block = -1;
        strcpy(inodes[i].name,"emptyfi");
    }
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);
    for (int i = 0; i <sb.num_blocks ; ++i) {
        dbs[i].next_block_num = -1;
    }

    FILE *file;
    file = fopen("mkfs","w+");
    fwrite(&sb,sizeof(struct superblock),1,file);

    for (int j = 0; j <sb.num_inodes ; ++j) {
        fwrite(&(inodes[j]), sizeof(struct inode),1,file);
    }
    for (int k = 0; k <sb.num_blocks ; ++k) {
        fwrite(&(dbs[k]), sizeof(struct disk_block),1,file);
    }
    fclose(file);
}

// load the file system
int mymount(){
    mof=malloc(sizeof(struct myopenfile) * MAX_FILES);
    for (int i = 0; i < MAX_FILES; ++i) {
        mof[i].myfd=-1;
        mof[i].flag=-1;
    }

    FILE *file;
    file = fopen ("mkfs","r");

    //superblock
    fread(&sb, sizeof(struct superblock),1,file);

    inodes=malloc(sizeof(struct inode) * sb.num_inodes);
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);

    fread(inodes, sizeof(struct inode), sb.num_inodes,file);
    fread(dbs, sizeof(struct disk_block), sb.num_blocks,file);

    return -1;
}
int myopen(const char *pathname,int flags){
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    for (int i = 0; i <sb.num_inodes ; ++i) {
        if (!strcmp(inodes[i].name, pathname)){
            int p = add_to_myopenfile(inodes[i],flags);
            if (p==-1){
                printf("There is no space available in myopenfile\n");
                return -1;
            }
            return i; // return the place of the inode
        }
    }
    //no pathname in inodes:
    int index = add_new_inode(pathname);
    if (index==-1){
        printf("There is no available inode\n");
    }
    printf("INDEX: %d\n",index);
    int p = add_to_myopenfile(inodes[index],flags);
    if (p==-1){
        printf("There is no available space in myopenfile\n");
        return -1;
    }
    return p;
}

int myclose(int myfd);
ssize_t myread(int myfd,void *buf,size_t count);
ssize_t mywrite(int myfd,const void *buf,size_t count);
void print_myfs(){
    printf("superblock info\n");
    printf("\tnum inodes %d\n",sb.num_inodes);
    printf("\tnum blocks %d\n",sb.num_blocks);
    printf("\tsize blocks %d\n",sb.size_blocks);
    printf("inodes\n");
    int i;
    for ( i = 0; i < sb.num_inodes ; ++i) {
        printf("\tsize: %d block: %d name: %s \n",inodes[i].size,inodes[i].first_block,inodes[i].name);
    }
    for ( i = 0; i < sb.num_blocks ; ++i) {
        printf("\tblock num: %d next block: %d \n",i,dbs->next_block_num);
    }

    printf("myopenfile\n");
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
    }else{
        for(i = 0; i <100 ; ++i) {
            printf("\topenfilenum: %d myfd: %d flag: %d\n",i,mof[i].myfd,mof[i].flag);

        }
    }


}