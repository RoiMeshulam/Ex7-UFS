#include "mystdio.h"



myFILE* myfopen(const char *restrict pathname, const char *mode){
    if (!strcmp(mode,"r")){
        int myfd = myopen(pathname,2);
        myFILE* file = malloc(sizeof(struct myFILE));
        file->fd = myfd;
        return file;
    }
    else if(!strcmp(mode,"r+")){
        int myfd =myopen(pathname,4);
        myFILE* file = malloc(sizeof(struct myFILE));
        file->fd = myfd;
        return file;
    }
    else if(!strcmp(mode,"w")){
        int myfd =myopen(pathname,3);
        myFILE* file = malloc(sizeof(struct myFILE));
        file->fd = myfd;
        return file;
    }
    else if(!strcmp(mode,"a")){
        int myfd =myopen(pathname,1);
        myFILE* file = malloc(sizeof(struct myFILE));
        file->fd = myfd;
        return file;
    }
    else{
        return NULL;
    }
    return NULL;
}

int myfclose(myFILE *stream){
    myclose(stream->fd);
}

ssize_t myfread(void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream){
    int readbytes = myread(stream->fd,(char*)ptr,nmemb*size);
    return readbytes;
}

ssize_t myfwrite(const void *restrict ptr, size_t size, size_t nmemb, myFILE *restrict stream){
//    printf(" i am hereeeeeeeeeeeeee\n");
//    printf("%d\n",stream->fd);
//    printf("%d\n",(int)(size*nmemb));
//    printf("%s\n",(char*)ptr);
    int writebytes = mywrite(stream->fd,(char*)ptr,size*nmemb);
//    printf("write bytes-----------> %d\n",writebytes);
    return writebytes;
}

int myfseek(myFILE *stream, long offset, int whence){
    off_t ans = mylseek(stream->fd,(off_t)offset,whence);
    return (int)ans;
}

int myfprintf(myFILE * stream, const char * format, ...) {
    int length = strlen(format);
    int num_args=0;
    int buf_index = 0;
    char buf[length+1000];
    memset(buf,'\0',(length+1000));
    if (length == 0) {
        return 0;
    }
    for (size_t i = 0; i < length ; ++i) {
        if (format[i] == '%'){
            num_args++;
        }
    }
    va_list ap;
    va_start(ap, num_args);
    for (size_t i = 0; i < length ; ++i) {
//        printf("%c\n",format[i]);
        if (format[i]=='%'){
            if (format[i+1] == 'd'){
                int temp =va_arg(ap, int);
                int bytes=sprintf(buf+buf_index,"%d",temp);
                buf_index+=bytes;
                i++;
            }
            else if (format[i+1] == 'c'){
//                printf("i am here\n");
                char temp =va_arg(ap, int);
                buf[buf_index]=temp;
                buf_index++;
                i++;
            }
            else if(format[i+1] == 'f'){
                float temp =va_arg(ap, double );
                int bytes=sprintf(buf+buf_index,"%f",temp);
                buf_index+=bytes;
                i++;
            }
            else{
                va_end(ap);
                return -1;
            }
        }else{
            buf[buf_index++]=format[i];
//            printf("%s\n",buf);
        }
    }
    va_end(ap);
//    printf("%s\n",buf);
//    printf("%d\n",buf_index);
    int bytes = myfwrite(buf,buf_index,1,stream); // make it append
//    printf("%d\n",bytes);
    if (bytes==-1){
        return -1;
    }
    return bytes;
}

int myfscanf(myFILE * stream, const char * format, ...){
    int size = get_size_inodes(stream->fd);
//    printf("my fscanf %d\n",size);
    char* data = (char*)malloc(size);
    char* number;
    int ret=0;
    mylseek(stream->fd,0,SEEK_SET);
    myread(stream->fd,data,get_size_inodes(stream->fd));
//    printf("str =%s\n", data);
    va_list vl;
    va_start( vl, format);
    int i = 0;
    int j=0;
    while (format && format[i])
    {
        if (format[i] == '%')
        {
            i++;
            switch (format[i])
            {
                case 'c':
                {
                    *(char *)va_arg( vl, char* ) = data[j];
                    j++;
                    ret++;
                    break;
                }
                case 'd':
                {
                    *(int *)va_arg( vl, int* ) =strtol(&data[j], &number, 10);
                    j+=number - &data[j];
                    ret++;
                    break;
                }
                case 'f':
                {
                    *(double *)va_arg( vl, double* ) =strtol(&data[j], &number, 10);
                    j+=number -  &data[j];
                    ret++;
                    break;
                }
            }
        }
        else
        {
            data[j] =format[i];
            j++;
        }
        i++;
    }
    va_end(vl);
    return ret;

}