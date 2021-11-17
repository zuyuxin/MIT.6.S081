#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"
#define MAXSIZE 1024
#define STDIN_FILE 0
#define MAXLEN 32
void run(char * argv[]){
    if(fork() == 0){
        exec(argv[0],argv);
        exit(0);
    }else{
        wait(0);
    }
}

int main(int argc, char* argv[])
{
    // xargs cmd <>
    if(argc < 2){
        printf("please using xargs command <args>\n");
    }
    char buf[MAXSIZE];
    char* params[MAXARG];
    int params_index = 0, arg_index = 0, max_rags = argc - 1;
    int i,read_len;
    for(i = 1 ; i < argc; i++) params[params_index++] = argv[i]; // argv[0] = 'xargs' , argv[1] = cmd, begin with 1,
    
    //在子进程中创建的空间，实际物理地址是和父进程不同的，但是虚拟地址是相同的
    while(1){
        while ( ( read_len = read(STDIN_FILE,buf,sizeof(buf) ) ) > 0 ) // 父进程和子进程都会执行这一步吗？
        {
            arg_index = 0;
            params_index = argc - 1;
            char * arg = (char*) malloc(MAXLEN);
            
            for(i = 0 ; i < read_len; i++){
                if(buf[i] == '\n' || buf[i] == ' '){
                    arg[arg_index] = 0;  
                    arg_index = 0; 
                    params[params_index++] = arg;
                    arg = (char*) malloc(MAXLEN);
                }else{
                    arg[arg_index++] = buf[i];
                }
            }
            
            arg[arg_index] = 0;
            params[params_index] = 0;
            max_rags = params_index > max_rags ? params_index : max_rags;
            run(params);
        }
        if(max_rags == argc - 1){
            run(params);
        }
        if(read_len <= 0){
            break;
        }
    }
    exit(0);
}


