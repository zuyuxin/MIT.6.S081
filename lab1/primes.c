#include"kernel/types.h"
#include"kernel/stat.h"
#include"user/user.h"

#define READEND 0
#define WRITEEND 1

void loop(int leftFd[2]){
    close(leftFd[WRITEEND]);
    int prime;
    if((read(leftFd[READEND],&prime,sizeof(int))==0)){
        //若leftFd[READEND]引用计数不为0，而缓冲区中没有数据，
        //则read会一直阻塞等待，直到有数据或者EOF标志（leftFd[READEND]引用计数为0）
        close(leftFd[READEND]);
        exit(0);
    } 
    printf("prime %d\n",prime);
    int rightFd[2];
    pipe(rightFd);
    if(fork() == 0){
        close(leftFd[READEND]);
        loop(rightFd);//rightFd中未必有东西
    }else{//父进程
        close(rightFd[READEND]);
        int num;
        //sleep(30);不需要sleep，因为read、wirte已经实现了同步机制，read的那一端会等待这一进程写完数据
        while(1){
            int n = read(leftFd[READEND],&num,sizeof(int));
            if(n == 0){
                break;
            }
            if(num % prime != 0){
                write(rightFd[WRITEEND],&num,sizeof(int));
            }
        }
        close(rightFd[WRITEEND]);
        close(leftFd[READEND]);
        wait((int*)0);
    }
    exit(0);
}


int main(int argc,char* argv[]){
    //通过pipe共享信息
    //父进程写，子进程读
    int fd[2];
    int i;
    pipe(fd);
    if(fork() == 0){
        sleep(20);
        //如何保证在父进程写入num之后再调用childProcess
        loop(fd);
    }else{
        close(fd[READEND]);
        for(i = 2 ; i < 36 ; i++){
            write(fd[WRITEEND],&i,sizeof(int));
        }
        close(fd[WRITEEND]);
        wait((int*)0); //等待子进程退出，回收子进程残余空间
    }
    exit(0);
}