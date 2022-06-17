#include "myfs.h"

int const freeBlock= -1;
int const usedBlock= -2;

//flags:
int const O_CREAT = 0; //create a regular file
int const O_APPEND = 1; // the file is opened in append mode
int const O_RDONLY = 2; // open the file for read only
int const O_WRONLY = 3; // open the file for write only
int const O_RDWR = 4; //open the file for both reading and writing

//whence:
int const SEEK_Begin = 0; // The file offset is set to offset bytes.
int const SEEK_Curr = 1; // The file offset is set to its current location plus offset bytes.
int const SEEK_End = 2; // The file offset is set to the size of the file plus offset bytes.

//global structs
struct superblock sb;
struct inode *inodes;
struct disk_block *dbs;
struct myopenfile *mof=NULL;

int get_size_inodes(int myfd) {
    return inodes[mof[myfd].myfd].datasize;
}

void clean_data(int myfd){
    int currblock = inodes[mof[myfd].myfd].first_block;
    while (currblock!= -2){
        memset(dbs[currblock].data,'\0',DATASIZE);
        currblock=dbs[currblock].next_block_num;
    }
    mof[myfd].curr_seek=0;
}

void sync_fs(const char *source){
    FILE *file;
    file = fopen(source,"w");
    fwrite(&sb,sizeof(struct superblock),1,file);

    for (int j = 0; j <sb.num_inodes ; ++j) {
        fwrite(&(inodes[j]), sizeof(struct inode),1,file);
    }
    for (int k = 0; k <sb.num_blocks ; ++k) {
        fwrite(&(dbs[k]), sizeof(struct disk_block),1,file);
    }
    fclose(file);
}

int allocate_more_blocks(int myfd , int numOfBlocks){
    int currBlock = inodes[mof[myfd].myfd].first_block;
    while (dbs[currBlock].next_block_num!=-2){
        currBlock=dbs[currBlock].next_block_num;
    }
    while (numOfBlocks>0){
        int empty = find_empty_block();
        if (empty==-1){
            printf("No available blocks in disk\n");
        }
        dbs[currBlock].next_block_num=empty;
        dbs[empty].next_block_num=-2;
        numOfBlocks--;
    }
    return 1;
}

int readFromBlocks(int myfd,int pos, char *buf , size_t count){
    int togo = pos / DATASIZE;
    int currBlock= inodes[mof[myfd].myfd].first_block;
    while (togo>0){
        currBlock = dbs[currBlock].next_block_num;
        togo--;
    }
    int seek = pos % DATASIZE;
    for (int i = 0; i < count ; ++i) {
       buf[i]=dbs[currBlock].data[(seek++)%DATASIZE];
    }
    return count; // count of bytes  read;

}

int writeToBlocks(int myfd , int pos ,char* data , int count){
    int togo = pos / DATASIZE;
    int currBlock= inodes[mof[myfd].myfd].first_block;
    while (togo>0){
        currBlock = dbs[currBlock].next_block_num;
        togo--;
    }
    int seek = pos % DATASIZE;
    for (int i = 0; i < count ; ++i) {
        if (seek==DATASIZE){
            seek=0;
        }
        dbs[currBlock].data[seek++]= data[i];
    }
    inodes[mof[myfd].myfd].datasize+=count;
    return count;
}

int find_empty_block(){
    int i;
    for (int i = 0; i < sb.num_blocks; ++i) {
        if (dbs[i].next_block_num==-1){
            dbs[i].next_block_num=-2; // in use
            return i;
        }
    }
    return -1;
}

void shorten_file(int block_num){
 int temp = dbs[block_num].next_block_num;
 if(temp>=0){
     shorten_file(temp);
 }
 dbs[block_num].next_block_num = -1;
}

void set_filesize(int myfd , int count){
    int tmp = count + sb.size_blocks -1;
    int num = tmp / sb.size_blocks;
    int bn = inodes[myfd].first_block;
    num--;
    while (num>0){
        int next_num =dbs[bn].next_block_num;
        if (next_num==-2){
            int empty=find_empty_block();
            if (empty==-1){
                printf("there is no space in the disk\n");
                return -1;
            }
            dbs[bn].next_block_num=empty;
            dbs[empty].next_block_num=-2;
        }
        bn=dbs[bn].next_block_num;
        num--;
    }
    shorten_file(bn);
    dbs[bn].next_block_num=-2;


}

int add_new_inode(const char* pathname){
    for (int i = 0; i <sb.num_inodes ; ++i) {
        if (inodes[i].first_block == freeBlock){
            strcpy(inodes[i].name,pathname); // check if path name has 8 byte only
            inodes[i].first_block=usedBlock; // in use
            return i;
        }
    }
    return -1; // return -1 if there is no available inode
}

int add_to_myopenfile(int index ,int flags){
    // Check if we have already opened the file
    for (int i = 0; i < MAX_FILES ; ++i) {
        if (inodes[index].isDir == 1 && index==mof[i].myfd){
            return i; // the pos in mof
        }
    }
    for (int i = 0; i <MAX_FILES ; ++i) {
        // the first empty place will be for the new open file
        if (inodes[index].isDir == 2){ // it is a file
            if (mof[i].myfd == -1){
                mof[i].myfd= index;
                mof[i].flag=flags;
                mof[i].curr_seek=0;
                return i;
            }
        }
        if (inodes[index].isDir == 1){ // it is a Dir
            if (mof[i].myfd == -1){
                mof[i].myfd= index;
                mof[i].flag=flags;
                mof[i].curr_seek = -1; // pos of currFD  in fds
                return i;
            }
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
    printf("SuperBlock information:\n");
    printf("sb.num_inodes : %d\n",sb.num_inodes);
    printf("sb.num_blocks : %d\n",sb.num_blocks);
    printf("sb.size_blocks : %d\n",sb.size_blocks);
    printf("\n");
    inodes=malloc(sizeof(struct inode) * sb.num_inodes);
    for (int i = 0; i <sb.num_inodes ; ++i) {
        inodes[i].number_of_blocks = -1;
        inodes[i].first_block = -1;
        strcpy(inodes[i].name,"emptyfi");
        inodes[i].datasize=0;
        inodes[i].isDir=0;
    }
    dbs = malloc(sizeof(struct disk_block) * sb.num_blocks);
    for (int i = 0; i <sb.num_blocks ; ++i) {
        dbs[i].next_block_num = -1;
        memset(dbs[i].data,'\0',DATASIZE);
    }

}

int mymount(const char *source, const char *target,const char *filesystemtype, unsigned long mountflags, const void *data){
    // there is available disk already
    strcpy(sb.disk_name,source);
    if (access( source, F_OK ) != -1){
        if(inodes==NULL && dbs == NULL){
            mymkfs(10000); // default size of my disk
        }

        mof=malloc(sizeof(struct myopenfile) * MAX_FILES);
        for (int i = 0; i < MAX_FILES; ++i) {
            mof[i].myfd = -1;
            mof[i].flag = -1;
            mof[i].curr_seek = -1;
        }
        char temp[100];
        strcpy(temp, target);
        char *token;
        const char s[2] = "/";
        token = strtok(temp, s);

        // making dir for my root directory
        int newInode = add_new_inode(token);
        int empty = find_empty_block();
        inodes[newInode].first_block=empty;
        inodes[newInode].number_of_blocks=1;
        inodes[newInode].isDir=1;
        inodes[newInode].datasize=0;
        int myfd = add_to_myopenfile(newInode,-1);
        struct mydirent *root = malloc(sizeof(struct mydirent));
        for (size_t i = 0; i < 12; i++) {
            root->fds[i]=-1;
        }
        strcpy(root->name,token); // first name in names[] is the dirname ("root")
        root->size=0;
        char *write_blocks = (char*)root;
        writeToBlocks(mof[myfd].myfd,0,write_blocks, sizeof(struct mydirent));
        inodes[newInode].datasize=sizeof(struct mydirent);
        free(root);

        token = strtok(NULL, s);
        // open all the dirs of target
        if (token!=NULL){
            myopendir(target);
        }
        sync_fs(source);
        return 1;
    }else{
        mymkfs(10000); // 10000 is my default size
        mof=malloc(sizeof(struct myopenfile) * MAX_FILES);
        for (int i = 0; i < MAX_FILES; ++i) {
            mof[i].myfd = -1;
            mof[i].flag = -1;
            mof[i].curr_seek = -1;
        }
        char temp[100];
        strcpy(temp, target);
        char *token;
        const char s[2] = "/";
        token = strtok(temp, s);
        // making dir for my root directory
        int newInode = add_new_inode(token);
        int empty = find_empty_block();
        inodes[newInode].first_block=empty;
        inodes[newInode].number_of_blocks=1;
        inodes[newInode].isDir=1;
        inodes[newInode].datasize=0;
        int myfd = add_to_myopenfile(newInode,-1);
        struct mydirent *root = malloc(sizeof(struct mydirent));
        for (size_t i = 0; i < 12; i++) {
            root->fds[i]=-1;
        }
        strcpy(root->name,token); // first name in names[] is the dirname ("root")
        root->size=0;
        char *write_blocks = (char*)root;
        writeToBlocks(mof[myfd].myfd,0,write_blocks, sizeof(struct mydirent));
        inodes[newInode].datasize=sizeof(struct mydirent);
        free(root);
        token = strtok(NULL, s);
        // open all the dirs of target
        if (token!=NULL){
            myopendir(target);
        }
        sync_fs(source);
        return 1;
    }
    return -1;
}

int myopen(const char *pathname,int flags){
    int k=0;

    if (mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    char str[100];
    char path_without_last[100];
    strcpy(str, pathname);
    char *token;
    const char s[2] = "/";
    token = strtok(str, s);
    char curr_p[12] = "";
    char last_p[12] = "";
    while (token != NULL) {
        strcpy(last_p, curr_p);
        strcpy(curr_p, token);
        token = strtok(NULL, s);
        if (token!=NULL){
            strcat(path_without_last,"/");
            strcat(path_without_last,curr_p);
        }
    }
    for (int i = 0; i <sb.num_inodes ; ++i) {
        if (!strcmp(inodes[i].name, curr_p)){
            if (inodes[i].isDir != 2 ){
                perror("Myopen can open only files");
                return -1;
            }
            int p = add_to_myopenfile(i,flags);
            if (p==-1){
                printf("There is no space available in myopenfile\n");
                return -1;
            }
            return p; // return the place of the inode
        }
    }

    //curr_name does not exist
    if (flags == O_RDONLY){ // These flags don't make a new file if it doesn't exist.
        return -1;
    }
//    printf("path: %s\n",path_without_last);
    myDIR *dirfd = myopendir(path_without_last);
//    printf("last dir fd is %d\n",dirfd->dirp);
//    printf("last dir pos is %d\n",dirfd->pos);
    if (dirfd->dirp == -1 ){
        perror("cannot open this dir\n");
        return -1;
    }

    int dirBlock = inodes[mof[dirfd->dirp].myfd].first_block;
//    printf("mof[dirfd->dirp].myfd = %d\n",mof[dirfd->dirp].myfd);
//    printf("dirblock = %d\n",dirBlock);
    struct mydirent* prevDir = (struct mydirent*)dbs[dirBlock].data;
    for (int i = 0; i < 12 ; ++i) {
        if (prevDir->fds[i]==-1){ // find fd empty in pervDir
            prevDir->fds[i]= add_new_inode(curr_p);
            if (prevDir->fds[i]==-1){
                return -1; // cannot create the inode
            }
            int empty = find_empty_block();
            if (empty==-1){
                printf("cannot find an empty block\n");
                return -1;
            }
            inodes[prevDir->fds[i]].first_block = empty;
            inodes[prevDir->fds[i]].isDir = 2; // isFile
            prevDir->size++;
            int p = add_to_myopenfile(prevDir->fds[i],flags);
            sync_fs(sb.disk_name);
//            printf("new check %d\n",p);
            return p;
        }
    }
//    printf("checkkkkkkkkkkkkkkkkkkkkkkk\n");
    return -1;
}

int myclose(int myfd){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    if (myfd<0){
        return -1;
    }
    if (mof[myfd].myfd>=0){
        mof[myfd].curr_seek = -1;
        mof[myfd].myfd = -1;
        mof[myfd].flag= -1;
        return 1;

    }
    return -1;

}

off_t mylseek(int myfd,off_t offset, int whence){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    if ((mof[myfd].myfd>=0)  && mof[myfd].flag!=O_CREAT && mof[myfd].flag != O_APPEND){
        if (whence == SEEK_SET){
            mof[myfd].curr_seek = (int)offset;
            return mof[myfd].curr_seek;
        }
        else if(whence == SEEK_CUR){
            mof[myfd].curr_seek += (int)offset;
            return mof[myfd].curr_seek;
        }
        else if(whence == SEEK_END){
            mof[myfd].curr_seek = (inodes[mof[myfd].myfd].number_of_blocks * DATASIZE) + (int)offset;
            return mof[myfd].curr_seek;
        }else{
            printf("whence is illegal\n");
            return -1;
        }
    }
    return -1; // check what happened if I have more than one fd in openfile
}

ssize_t myread(int myfd,char *buf,size_t count){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    if (mof[myfd].myfd>=0 && ((mof[myfd].flag == O_RDONLY)||(mof[myfd].flag==O_RDWR))){
        if (inodes[mof->myfd].first_block==-2) {
            printf("There is no data to read\n");
            return -1;
        }else{
            int bytes=readFromBlocks(inodes[mof[myfd].myfd].first_block,mof[myfd].curr_seek,buf,count);
            mof[myfd].curr_seek+=bytes;
            return bytes; // how mauch i read
        }
    }
    return -1;
}

ssize_t mywrite(int myfd,const void *buf,size_t count){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    if((mof[myfd].myfd>=0) && (mof[myfd].flag == O_RDWR)) { // read and write mod
        if (inodes[mof[myfd].myfd].first_block == -2) { // didn't write yet
            inodes[mof[myfd].myfd].first_block = find_empty_block();
            set_filesize(mof[myfd].myfd, count);
            int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block, mof[myfd].curr_seek, buf, count);
            mof[myfd].curr_seek+=bytes;
            inodes[mof[myfd].myfd].number_of_blocks = (count + DATASIZE) / DATASIZE;
//            printf("1) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//            printf("1) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//            printf("1) curr_seek: %d\n", mof[myfd].curr_seek);
            sync_fs(sb.disk_name);
            return bytes;
        } else {
            if ((mof[myfd].curr_seek + count) > (inodes[mof[myfd].myfd].number_of_blocks * DATASIZE)) {
                int placeInLastBlock = DATASIZE - (mof[myfd].curr_seek % DATASIZE);
                int numOfBlockToAlloc = count - placeInLastBlock;
                numOfBlockToAlloc = numOfBlockToAlloc / DATASIZE;
                allocate_more_blocks(myfd, numOfBlockToAlloc);
                int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block, mof[myfd].curr_seek, buf, count);
                mof[myfd].curr_seek+=bytes;
                inodes[mof[myfd].myfd].number_of_blocks = (mof[myfd].curr_seek + DATASIZE) / DATASIZE;
//                printf("2) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//                printf("2) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//                printf("2) curr_seek: %d\n", mof[myfd].curr_seek);
                sync_fs(sb.disk_name);
                return bytes;
            }else{// don't need to resize
                int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block,mof[myfd].curr_seek,buf,count);
                mof[myfd].curr_seek+=bytes;
                inodes[mof[myfd].myfd].number_of_blocks = (mof[myfd].curr_seek + DATASIZE) /DATASIZE;
//                printf("3) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//                printf("3) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//                printf("3) curr_seek: %d\n",  mof[myfd].curr_seek);
                sync_fs(sb.disk_name);
                return bytes;
            }
        }
    }
    else if((mof[myfd].myfd>=0) && (mof[myfd].flag == O_WRONLY)) {
        if (inodes[mof[myfd].myfd].first_block == -2) { // i didnt write yet
            inodes[mof[myfd].myfd].first_block = find_empty_block();
            set_filesize(mof[myfd].myfd, count);
            int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block, mof[myfd].curr_seek, buf, count);
            mof[myfd].curr_seek+=bytes;
            inodes[myfd].number_of_blocks = (DATASIZE + count) / DATASIZE;
//            printf("4) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//            printf("4) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//            printf("4) curr_seek: %d\n", mof[myfd].curr_seek);
            sync_fs(sb.disk_name);
            return bytes;
        } else { // i write and i need to erase what i write and write again from the firstBlock
            clean_data(inodes[mof[myfd].myfd].first_block);
            set_filesize(myfd, count);
            int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block, mof[myfd].curr_seek, buf, count);
            mof[myfd].curr_seek+=bytes;
            inodes[myfd].number_of_blocks = (count + DATASIZE) / DATASIZE;
//            printf("5) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//            printf("5) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//            printf("5) curr_seek: %d\n", mof[myfd].curr_seek);
            sync_fs(sb.disk_name);
            return bytes;
        }

    }
    else if((mof[myfd].myfd>=0) && mof[myfd].flag == O_APPEND){
        if (inodes[mof[myfd].myfd].first_block==-2){ //didn't write yet
            inodes[mof[myfd].myfd].first_block=find_empty_block();
            set_filesize(mof[mof[myfd].myfd].myfd,count);
            int bytes= writeToBlocks(inodes[mof[myfd].myfd].first_block,mof[myfd].curr_seek,buf,count);
            mof[myfd].curr_seek+=bytes;
            inodes[mof[myfd].myfd].number_of_blocks=(count+DATASIZE)/DATASIZE ;
//            printf("6) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//            printf("6) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//            printf("6) curr_seek: %d\n",  mof[myfd].curr_seek);
            sync_fs(sb.disk_name);
            return bytes;
        }else{ // write from the end
            if (inodes[mof[myfd].myfd].datasize + count > (inodes[mof[myfd].myfd].number_of_blocks * DATASIZE)){
                //need to resize
                int placeInLastBlock = DATASIZE - (inodes[mof[myfd].myfd].datasize % DATASIZE);
                int numOfBlockToAlloc = count - placeInLastBlock;
                numOfBlockToAlloc = numOfBlockToAlloc/DATASIZE;
                allocate_more_blocks(mof[myfd].myfd,numOfBlockToAlloc);
                int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block,mof[myfd].curr_seek,buf,count);
                mof[myfd].curr_seek+=count;
                inodes[myfd].number_of_blocks = (inodes[mof[myfd].myfd].datasize + DATASIZE) /DATASIZE;
//                printf("7) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//                printf("7) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//                printf("7) curr_seek: %d\n",  mof[myfd].curr_seek);
                sync_fs(sb.disk_name);
                return bytes;
            }else{
                mylseek(myfd,-1,SEEK_END);
                int bytes = writeToBlocks(inodes[mof[myfd].myfd].first_block,mof[myfd].curr_seek,buf,count);
                mof[myfd].curr_seek+=count;
                inodes[mof[myfd].myfd].number_of_blocks = (inodes[myfd].datasize + DATASIZE) /DATASIZE;
//                printf("8) datasize: %d\n", inodes[mof[myfd].myfd].datasize);
//                printf("8) number_of_blocks: %d\n", inodes[mof[myfd].myfd].number_of_blocks);
//                printf("8) curr_seek: %d\n",  mof[myfd].curr_seek);
                sync_fs(sb.disk_name);
                return bytes;
            }
        }
    }
    else {
        return -1;
    }
}

myDIR* myopendir(const char *name){
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
        return NULL;
    }
    char str[100];
    char path_without_last[100];
    strcpy(str, name);
    char *token;
    const char s[2] = "/";
    token = strtok(str, s);
    char curr_p[12] = "";
    char last_p[12] = "";
    while (token != NULL) {
        strcpy(last_p, curr_p);
        strcpy(curr_p, token);
        token = strtok(NULL, s);
        if (token != NULL){
            strcat(path_without_last,"/");
            strcat(path_without_last,curr_p);
        }
    }
    for (int i = 0; i < sb.num_inodes; i++) {
        if (!strcmp(inodes[i].name, curr_p)) {
            if (inodes[i].isDir != 1) {
                perror("It wasn't a dir\n");
                return NULL;
            }
//            printf("i have a dir now add to my openfile the new dir\n");
            int dirfd = add_to_myopenfile(i,-1);
//            printf("dirfd %d\n",dirfd);
            if (dirfd == -1){
                perror("couldn't add it to myopenfile\n");
                return NULL;
            }
            myDIR *ans =(myDIR*) malloc(sizeof (myDIR));
            ans->dirp = dirfd; // pos in mof
//            printf("dirfd %d\n",dirfd);
//            printf("ans dirp is %d\n",ans->dirp);
            ans->pos = -1;
            return ans;
        }
    }
    //making new dir
    myDIR* temp = myopendir(path_without_last);
    if (temp->dirp == -1) {
        perror("ERROR");
        return NULL;
    }
    if (inodes[mof[temp->dirp].myfd].isDir != 1) {
        perror("It wasn't a dir\n");
        return NULL;
    }
    int currBlock = inodes[mof[temp->dirp].myfd].first_block;
    struct mydirent *prevDir = (struct mydirent *) dbs[currBlock].data;
    int inode_pos = add_new_inode(curr_p);
//    printf("new inode pos: %d\n",inode_pos);
    if (inode_pos==-1){
        perror("cannot make new dir\n");
    }
    int empty = find_empty_block();
    if (empty==-1) {
        perror("There is no more blocks");
    }
    inodes[inode_pos].first_block=empty;
    inodes[inode_pos].isDir = 1; //isDir
    inodes[inode_pos].number_of_blocks = 1;
    for (int i = 0; i < 12 ; ++i) {
        if (prevDir->fds[i]==-1){
            prevDir->fds[i]= inode_pos;
            prevDir->size++;
            break;
        }
    }
    int dirpfd = add_to_myopenfile(inode_pos,-1);
    struct mydirent *newdir = malloc(sizeof(struct mydirent));
    newdir->size = 0;
    newdir->fds[0]= inode_pos; // fds[0] will hold his own fd;
    newdir->fds[1]= temp->dirp; // fds[1] will hold his previous dir fd;
    for (size_t i = 2; i < 12; i++) {
        newdir->fds[i] = -1;
    }
    char *buf = (char *) newdir;
    writeToBlocks(inodes[inode_pos].first_block,0,buf, sizeof(struct mydirent));
    strcpy(newdir->name, name); //the name of the dir
    sync_fs(sb.disk_name);
    temp-> dirp = dirpfd;
    temp -> pos = -1;
    return temp;
}

struct mydirent *myreaddir(struct myDIR* dirp){
    struct mydirent *currDir=malloc(sizeof(struct mydirent));
    for (int i = 0; i < MAX_FILES ; ++i) {
        if (mof[i].myfd == dirp->dirp){
            if (inodes[dirp->dirp].isDir != 1){
                perror("It wasn't a dir\n");
                return NULL;
            }
            int currBlock = inodes[dirp->dirp].first_block;
            currDir = (struct mydirent *) dbs[currBlock].data;
            dirp->pos++;
            for (int j = dirp->pos; j < 12 ; ++j) {
                if (currDir->fds[j] != -1){
                    if (!strcmp(inodes[currDir->fds[j]].name,inodes[dirp->dirp].name)){
                        strcpy(currDir->name,".");
                    }
                    else if(j==1){
                        strcpy(currDir->name,"..");
                    }
                    else{
                        strcpy(currDir->name,inodes[currDir->fds[j]].name);
                    }
                    return currDir;
                }
            }
        }
    }
    return NULL;
}

int myclosedir(int dirp){
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
        return NULL;
    }
    for (int i = 0; i < MAX_FILES; ++i) {
        if (mof[i].myfd== dirp){
            mof[i].myfd= -1;
            mof[i].curr_seek = -1;
            mof[i].flag = -1;
            return 1;
        }
    }
    return -1;
}

void print_myfs(){
    printf("superblock info\n");
    printf("\tnum inodes %d\n",sb.num_inodes);
    printf("\tnum blocks %d\n",sb.num_blocks);
    printf("\tsize blocks %d\n",sb.size_blocks);
    printf("\tinodes\n");
    int i;
    for ( i = 0; i < sb.num_inodes ; ++i) {
        printf("\tNunber of blocks: %d fblock: %d name: %s isDir: %d Datasize: %d\n",inodes[i].number_of_blocks,inodes[i].first_block,inodes[i].name,inodes[i].isDir,inodes[i].datasize);
        if (inodes[i].isDir==1){
            struct mydirent *curr_dir = (struct mydirent*)dbs[inodes[i].first_block].data;
            printf("\tfds: ");
            for (int j = 0; j < 12 ; ++j) {
                printf(" %d",curr_dir->fds[j]);
            }
            printf("\n");
        }
    }
    for ( i = 0; i < sb.num_blocks ; ++i) {
        printf("\tblock num: %d next block: %d \n",i,dbs[i].next_block_num);
    }

    printf("\tmyopenfile\n");
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
    }else{
        for(i = 0; i <100 ; ++i) {
            printf("\topenfilenum: %d myfd: %d flag: %d currseek: %d isDir %d \n",i,mof[i].myfd,mof[i].flag, mof[i].curr_seek , inodes[mof[i].myfd].isDir);

        }
    }
    for (int l = 0; l <sb.num_blocks ; ++l) {
        printf("num of block : %d , data : %s\n",l,dbs[l].data);
    }




}