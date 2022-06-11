#include "mymkfs.h"

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

void clean_data(int firstBlock){
    int currblock = firstBlock;
    while (currblock!= -2){
        memset(dbs[currblock].data,'\0',DATASIZE);
        currblock=dbs[currblock].next_block_num;
    }
}

void sync_fs(){
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

int allocate_more_blocks(int myfd , int numOfBlocks){
    int currBlock = inodes[myfd].first_block;
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
    printf("read from blocks\n");
    printf("pos %d\n",pos);
    int togo = pos / DATASIZE;
    printf("togo %d\n",togo);
    int currBlock= inodes[myfd].first_block;
    while (togo>0){
        currBlock = dbs[currBlock].next_block_num;
        togo--;
    }
    int seek = pos % DATASIZE;
    printf("first seek %d\n",seek);
    for (int i = 0; i < count ; ++i) {
        if (seek==DATASIZE){
            seek=0;
        }
//        printf("data size %d\n",inodes[myfd].datasize);
//        if (inodes[myfd].datasize==seek){ //there is no data to read anymore
//            return pos +count;
//        }
        buf[i]=dbs[currBlock].data[seek++];
    }
    return pos + count;

}

int writeToBlocks(int firstBlock , int pos ,char* data , int count){
    int togo = pos / DATASIZE;
    int currBlock= firstBlock;
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
    return pos + count;

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
    printf("add new inode\n");
    for (int i = 0; i <sb.num_inodes ; ++i) {
        if (inodes[i].first_block == freeBlock){
            strcpy(inodes[i].name,pathname); // check if path name has 8 byte only
            printf("inode name : %s\n",inodes[i].name);
            inodes[i].first_block=usedBlock; // in use
            return i;
        }
    }
    return -1; // return -1 if there is no available inode
}

int add_to_myopenfile(int index ,int flags){
    // Check if we have already opened the file
    for (int i = 0; i < MAX_FILES ; ++i) {
        if (inodes[index].isDir==1 && index==mof[i].myfd){
            return index;
        }
    }
    for (int i = 0; i <MAX_FILES ; ++i) {
        // the first empty place will be for the new open file
        if (inodes[index].isDir == 2){ // it is a file
            if (mof[i].myfd == -1){
                mof[i].myfd= index;
                mof[i].flag=flags;
                mof[i].curr_seek=0;
                return mof[i].myfd;
            }
        }
        if (inodes[index].isDir == 1){ // it is a Dir
            if (mof[i].myfd == -1){
                mof[i].myfd= index;
                mof[i].flag=flags;
                mof[i].curr_seek = -1; // pos of currFD  in fds
                return mof[i].myfd;
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
    printf("sb.num_inodes : %d\n",sb.num_inodes);
    printf("sb.num_blocks : %d\n",sb.num_blocks);
    printf("sb.size_blocks : %d\n",sb.size_blocks);

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

    //making root dir
    int newInode = add_new_inode("root");
    printf("%d\n",newInode);
    int empty = find_empty_block();
    inodes[newInode].first_block=empty;
    inodes[newInode].number_of_blocks=1;
    inodes[newInode].isDir=1;
    struct mydirent *root = malloc(sizeof(struct mydirent));
    for (size_t i = 0; i < 12; i++) {
        root->fds[i]=-1;
    }
    strcpy(root->name,"root"); // first name in names[] is the dirname ("root")
    root->size=0;
    char *write_blocks = (char*)root;
    writeToBlocks(inodes[newInode].first_block,0,write_blocks, sizeof(struct mydirent));
    inodes[newInode].datasize=sizeof(struct mydirent);
    free(root);
    sync_fs();
}

// load the file system
int mymount(){
    //initialization of myopenfile
    mof=malloc(sizeof(struct myopenfile) * MAX_FILES);
    for (int i = 0; i < MAX_FILES; ++i) {
        mof[i].myfd = -1;
        mof[i].flag = -1;
        mof[i].curr_seek = -1;
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
            return i; // return the place of the inode
        }
    }

    //curr_name does not exist
    if (flags==O_RDONLY || flags == O_RDWR){ // These flags don't make a new file if it doesn't exist.
        return -1;
    }
    myDIR *dirfd = myopendir(path_without_last);
    if (dirfd->dirp == -1 ){
        perror("cannot open this dir\n");
        return -1;
    }
    int dirBlock = inodes[dirfd->dirp].first_block;
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
            sync_fs();
            return p;
        }
    }
    return -1;
}

int myclose(int myfd){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    for (int i = 0; i < MAX_FILES ; ++i) {
        if (mof[i].myfd == myfd){
            mof[i].myfd =  -1;
            mof[i].flag = -1;
            mof[i].curr_seek = -1;
            return 1;
        }
    }
    printf("their is no openfile with this myfd\n");
    return -1; //

}

off_t mylseek(int myfd,off_t offset, int whence){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    for (int i = 0; i <MAX_FILES ; ++i) {
        if (mof[i].myfd==myfd && mof[i].flag!=O_CREAT && mof[i].flag != O_APPEND){
            if (whence == SEEK_SET){
                mof[i].curr_seek = (int)offset;
                return mof[i].curr_seek;
            }
            else if(whence == SEEK_CUR){
                mof[i].curr_seek += (int)offset;
                return mof[i].curr_seek;
            }
            else if(whence == SEEK_END){
                mof[i].curr_seek = (inodes[myfd].number_of_blocks * DATASIZE) + (int)offset;
                return mof[i].curr_seek;
            }else{
                printf("whence is illegal\n");
                return -1;
            }
        }
    }
    return -1; // check what happened if I have more than one fd in openfile
}

ssize_t myread(int myfd,char *buf,size_t count){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    for (int i = 0; i <MAX_FILES ; ++i) {
        if ((mof[i].myfd == myfd) && ((mof[i].flag == O_RDONLY)||(mof[i].flag==O_RDWR))){
            printf("i find a fd %d \n",myfd);
            if (inodes[myfd].first_block==-2) {
                printf("There is no data to read\n");
                return -1;
            } else{
                int seek=readFromBlocks(inodes[myfd].first_block,mof[i].curr_seek,buf,count);
                mof[i].curr_seek=seek;
                return count; // how mauch i read
            }
        }
        else{
            continue; // maybe there is another fd with read flag
        }
    }

    return -1;
}

ssize_t mywrite(int myfd,const void *buf,size_t count){
    if(mof==NULL){
        printf("my mount didn't happen yet\n");
        return -1;
    }
    for (int i = 0; i < MAX_FILES ; ++i) {
        if ((mof[i].myfd == myfd) && (mof[i].flag == O_RDWR)){ //read and write mod
            printf("read write mod\n");
            if (inodes[myfd].first_block==-2){ // didn't write yet
                inodes[myfd].first_block=find_empty_block();
                set_filesize(myfd,count);
                inodes[myfd].datasize= writeToBlocks(inodes[myfd].first_block,mof[i].curr_seek,buf,count);
                inodes[myfd].number_of_blocks=(count+DATASIZE)/DATASIZE ;
                mof[i].curr_seek=count;
                printf("1) datasize: %d\n", inodes[myfd].datasize);
                printf("1) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                printf("1) curr_seek: %d\n",  mof[i].curr_seek);
                sync_fs();
                return count;
            } else{
                printf("i have a file exist\n");
                if ((mof[i].curr_seek + count) > (inodes[myfd].number_of_blocks * DATASIZE)){
                    printf("need to resize\n");
                    int placeInLastBlock = DATASIZE - (mof[i].curr_seek % DATASIZE);
                    int numOfBlockToAlloc = count - placeInLastBlock;
                    numOfBlockToAlloc = numOfBlockToAlloc/DATASIZE;
                    allocate_more_blocks(myfd,numOfBlockToAlloc);
                    int curr_seek = writeToBlocks(inodes[myfd].first_block,mof[i].curr_seek,buf,count);
                    mof[i].curr_seek=curr_seek;
                    inodes[myfd].number_of_blocks = (mof[i].curr_seek + DATASIZE) /DATASIZE;
                    inodes[myfd].datasize = curr_seek;
                    printf("2) datasize: %d\n", inodes[myfd].datasize);
                    printf("2) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                    printf("2) curr_seek: %d\n",  mof[i].curr_seek);
                    sync_fs();
                    return count;

                } else{ // don't need to resize
                    printf("don't need to resize\n");
                    int curr_seek = writeToBlocks(inodes[myfd].first_block,mof[i].curr_seek,buf,count);
                    mof[i].curr_seek=curr_seek;
//                    if (mof[i].curr_seek > inodes[myfd].datasize){
//                        inodes[myfd].datasize = mof[i].curr_seek;
//                    }
                    inodes[myfd].number_of_blocks = (mof[i].curr_seek + DATASIZE) /DATASIZE;
                    printf("3) datasize: %d\n", inodes[myfd].datasize);
                    printf("3) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                    printf("3) curr_seek: %d\n",  mof[i].curr_seek);
                    return count;

                }
            }
        }else if((mof[i].myfd == myfd) && (mof[i].flag == O_WRONLY)){
            if (inodes[myfd].first_block==-2){ // i didnt write yet
                inodes[myfd].first_block=find_empty_block();
                set_filesize(myfd,count);
                int ans = writeToBlocks(inodes[myfd].first_block,0,buf,count);
                mof[i].curr_seek=count;
                inodes[myfd].number_of_blocks= (DATASIZE+count)/DATASIZE;
                inodes[myfd].datasize = count;
                printf("4) datasize: %d\n", inodes[myfd].datasize);
                printf("4) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                printf("4) curr_seek: %d\n",  mof[i].curr_seek);
                sync_fs();
                return count;

            }else{ // i write and i need to erase what i write and write again from the firstBlock
                clean_data(inodes[myfd].first_block);
                set_filesize(myfd,count);
                int ans = writeToBlocks(inodes[myfd].first_block,0,buf,count);
                mof[i].curr_seek=count;
                inodes[myfd].number_of_blocks= (count+DATASIZE)/DATASIZE;
                inodes[myfd].datasize=count;
                printf("5) datasize: %d\n", inodes[myfd].datasize);
                printf("5) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                printf("5) curr_seek: %d\n",  mof[i].curr_seek);
                sync_fs();
                return count;
            }
        }else if(mof[i].myfd == myfd && mof[i].flag == O_APPEND){
            if (inodes[myfd].first_block==-2){ //didn't write yet
                inodes[myfd].first_block=find_empty_block();
                set_filesize(myfd,count);
                writeToBlocks(inodes[myfd].first_block,0,buf,count);
                inodes[myfd].number_of_blocks=(count+DATASIZE)/DATASIZE ;
                mof[i].curr_seek=count;
                inodes[myfd].number_of_blocks= (count+DATASIZE)/DATASIZE;
                inodes[myfd].datasize=count;
                printf("6) datasize: %d\n", inodes[myfd].datasize);
                printf("6) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                printf("6) curr_seek: %d\n",  mof[i].curr_seek);
                sync_fs();
                return count;
            }else{ // write from the end
                if (inodes[myfd].datasize + count > (inodes[myfd].number_of_blocks * DATASIZE)){
                    //need to resize
                    int placeInLastBlock = DATASIZE - (inodes[myfd].datasize % DATASIZE);
                    int numOfBlockToAlloc = count - placeInLastBlock;
                    numOfBlockToAlloc = numOfBlockToAlloc/DATASIZE;
                    allocate_more_blocks(myfd,numOfBlockToAlloc);
                    int curr_seek = writeToBlocks(inodes[myfd].first_block,mof[i].curr_seek,buf,count);
                    mof[i].curr_seek=curr_seek;
                    inodes[myfd].number_of_blocks = (mof[i].curr_seek + DATASIZE) /DATASIZE;
                    inodes[myfd].datasize = curr_seek;
                    printf("7) datasize: %d\n", inodes[myfd].datasize);
                    printf("7) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                    printf("7) curr_seek: %d\n",  mof[i].curr_seek);
                    sync_fs();
                    return count;
                }else{
                    int curr_seek = writeToBlocks(inodes[myfd].first_block,inodes[myfd].datasize,buf,count);
                    mof[i].curr_seek=curr_seek;
//                    inodes[myfd].number_of_blocks = (inodes[myfd].datasize + DATASIZE) /DATASIZE;
                    // doesnt need to change the datasize
                    printf("8) datasize: %d\n", inodes[myfd].datasize);
                    printf("8) number_of_blocks: %d\n", inodes[myfd].number_of_blocks);
                    printf("8) curr_seek: %d\n",  mof[i].curr_seek);
                    sync_fs();
                    return count;
                }
            }
        }else{ // O_CREAT or O_RDONLY
            continue; // keep search in mof if i have fd with flags that support writing
        }
    }

    // sync it to mkfs:

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


    //there is no file descriptor in myOpenFile
    return -1;
}

myDIR* myopendir(const char *name){
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
        return NULL;
    }
//    printf("%s\n",name);
    char str[100];
    char path_without_last[100];
    strcpy(str, name);
    char *token;
    const char s[2] = "/";
    token = strtok(str, s);
    char curr_p[12] = "";
    char last_p[12] = "";
    while (token != NULL) {
//        printf("i am still in the loop\n");
        strcpy(last_p, curr_p);
        strcpy(curr_p, token);
        token = strtok(NULL, s);
//        printf("%s\n",curr_p);
//        printf("%s\n",last_p);
//        printf("%d\n",token==NULL);
        if (token != NULL){
            strcat(path_without_last,"/");
            strcat(path_without_last,curr_p);
        }
    }
//    printf("out of the loop\n");
    for (int i = 0; i < sb.num_inodes; i++) {
        if (!strcmp(inodes[i].name, curr_p)) {
//            printf("%s\n",curr_p);
            if (inodes[i].isDir != 1) {
                perror("It wasn't a dir\n");
                return NULL;
            }
//            printf("%d is the index i send to myopenfile\n",i);
            int dirfd = add_to_myopenfile(i,-1);
//            printf("%d\n", dirfd);
            if (dirfd == -1){
                perror("couldn't add it to myopenfile\n");
                return NULL;
            }
            myDIR *ans = malloc(sizeof (myDIR));
            ans->dirp=dirfd;
            ans->pos=-1;
            return ans;
        }
    }
    //making new dir
    myDIR* temp = myopendir(path_without_last);
    if (temp->dirp == -1) {
        perror("ERROR");
        return NULL;
    }
    if (inodes[temp->dirp].isDir != 1) {
        perror("It wasn't a dir\n");
        return NULL;
    }
    int currBlock = inodes[temp->dirp].first_block;
    struct mydirent *prevDir = (struct mydirent *) dbs[currBlock].data;
    int new_dirfd = add_new_inode(curr_p);
    if (new_dirfd==-1){
        perror("cannot make new dir\n");
    }
    int empty = find_empty_block();
    if (empty==-1) {
        perror("There is no more blocks");
    }
    inodes[new_dirfd].first_block=empty;
    inodes[new_dirfd].isDir = 1; //isDir
    inodes[new_dirfd].number_of_blocks = 1;
    for (int i = 0; i < 12 ; ++i) {
        if (prevDir->fds[i]==-1){
            prevDir->fds[i]= new_dirfd;
            prevDir->size++;
        }
    }
    struct mydirent *newdir = malloc(sizeof(struct mydirent));
    newdir->size = 0;
    for (size_t i = 0; i < 12; i++) {
        newdir->fds[i] = -1;
    }
    char *buf = (char *) newdir;
    writeToBlocks(inodes[new_dirfd].first_block,0,buf, sizeof(struct mydirent));
    strcpy(newdir->name, name);
    sync_fs();
    temp-> dirp = new_dirfd;
    temp -> pos = -1;
    add_to_myopenfile(temp->dirp,-1);
    return temp;
}

struct mydirent *myreaddir(struct myDIR* dirp){
//    printf("in myreaddir dirp: %d pos: %d \n",dirp->dirp,dirp->pos);
    struct mydirent *currDir=malloc(sizeof(struct mydirent));
    for (int i = 0; i < MAX_FILES ; ++i) {
        if (mof[i].myfd == dirp->dirp){
//            printf("i found fd like dirp\n");
            if (inodes[dirp->dirp].isDir != 1){
                perror("It wasn't a dir\n");
                return NULL;
            }
            int currBlock = inodes[dirp->dirp].first_block;
            currDir = (struct mydirent *) dbs[currBlock].data;
            dirp->pos++;
            for (int j = dirp->pos; j < 12 ; ++j) {
//                printf("in for loop\n");
                if (currDir->fds[j] != -1){
//                    printf("i found fd in fds index %d the name is: %s \n",j,inodes[currDir->fds[j]].name);
                    strcpy(currDir->name,inodes[currDir->fds[j]].name);
//                    printf("curr name is %s \n", currDir->name);
//                    printf("%p\n",currDir);
                    return currDir;
                }
            }
//            return currDir;
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
    printf("inodes\n");
    int i;
    for ( i = 0; i < sb.num_inodes ; ++i) {
        printf("\tNunber of blocks: %d fblock: %d name: %s isDir: %d\n",inodes[i].number_of_blocks,inodes[i].first_block,inodes[i].name,inodes[i].isDir);
    }
    for ( i = 0; i < sb.num_blocks ; ++i) {
        printf("\tblock num: %d next block: %d \n",i,dbs[i].next_block_num);
    }

    printf("myopenfile\n");
    if (mof==NULL){
        printf("my mount didn't happen yet\n");
    }else{
        for(i = 0; i <100 ; ++i) {
            printf("\topenfilenum: %d myfd: %d flag: %d currseek: %d\n",i,mof[i].myfd,mof[i].flag, mof[i].curr_seek);

        }
    }
    for (int l = 0; l <sb.num_blocks ; ++l) {
        printf("num of block : %d , data : %s\n",l,dbs[l].data);
    }

//    struct mydirent *dir = (struct mydirent*)dbs[0].data;
//    struct mydirent *currdir = (struct mydirent *) dbs[d_b].data;
//    char buf[512];
//    readFromBlocks(0,0,buf, sizeof(struct mydirent));
//    printf("%d\n",dir->size);
//    for (int j = 0; j < 12; ++j) {
//        printf("%d\n",dir->fds[j]);
//    }



}