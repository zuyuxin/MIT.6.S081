/*
the ping-pong program
the requirements are:

(1) 父进程发送一个字节数据给子进程，子进程接收后打印"<{子进程的pid}>:received ping"
(2) 子进程写数据到管道中;父进程读取来自子进程的数据，打印"<{父进程的pid}>:received pong"，并且退出。
(3) 将实现写在user/pingpong.c中

*/
#include<kernel/types.h>
#include<user/user.h>
#define READ 0
#define WRITE 1
/*
    此题的目的是学习父进程与子进程之间的双向消息传输，要求父进程向子进程传输数据，子进程按照给定的格式打印数据以及自己的pid。子进程向父进程传输数据，父进程按给定的格式打印数据以及自己的pid
    进程之间的数据传输可以通过管道实现，一个进程向管道写数据，另一个进程从管道读数据。
    实现这个需求，需要创建两个管道
*/
/*
    关于pipe()函数
*/
int 
main(int argc, char * argv[]){
    int pipe1[2],pipe2[2];
    pipe(pipe1);pipe(pipe2);
    int buf[1];
    if(fork() == 0){
        close(pipe1[WRITE]);
        close(pipe2[READ]);

        read(pipe1[READ],buf,1);
        printf("%d: received ping\n",getpid());
        write(pipe2[WRITE],"Dad",3);

        close(pipe1[READ]);
        close(pipe2[WRITE]);
    }else{
        close(pipe1[READ]);
        close(pipe2[WRITE]);

        write(pipe1[WRITE],"Son",3);
        read(pipe2[READ],buf,1);
        printf("%d: received pong\n",getpid());
        
        close(pipe1[WRITE]);
        close(pipe2[READ]);
    }
    exit(0);
}

/*
一些要点
(1) int pipe(int fd[]) 接收的参数是大小为2的int类型数组
(2) pipe会创建两个文件描述符，并写入到实参中，fd[0]只能读，fd[1]只能写 (或许可以看成是指向一个缓冲区的两个文件描述符，一个操控读指针，一个操控写指针)
(3) 管道内实现了阻塞机制，当读进程从fd[0]中读数据时，若缓冲区为空，则改进程会一直等待，直到缓冲区中有数据或者fd[1]的引用计数为0（此时读进程会收到EOF信号，并且不再等待）


*/

/*
int 
main(int argc , char * argv[]){
    int fd[2];
    pipe(fd);
    char data[100];
    
    //write(fd[1],"a",1);
    //close(fd[1])只有当缓冲区没有数据的时候，才有作用
    //close(fd[1]);//在缓冲区设置EOF标志，防止read还需要再读数据时，缓冲区为空后阻塞等待，这里不会阻塞等待，因为read只读一个字节，如果读两个字节，如果不close(fd[1])就会阻塞等待，因为在读之前缓冲区只有一个字节的数据
    //read(fd[0],data,sizeof(data));
    close(fd[0]);//当fd[0]引用计数为0时，写数据无用
    write(fd[1],"a",1);
    read(fd[0],data,1);
    printf("test 1 read data: %s",data);
    //close(fd[1]);
    exit(0);
}
*/

