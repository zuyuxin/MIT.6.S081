#include<kernel/types.h>
#include<user/user.h>
int 
main(int argc, char* argv[])
{
    if(argc != 2){
	printf("you must input one argument!\n");
        exit(1);
    }
    int time = atoi(argv[1]);
    printf("sleep %d s\n",time);
    sleep(time);
    exit(0);
}






