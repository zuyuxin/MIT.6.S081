# Lab2：system calls

# 一、前言
这一节的实验开始就要深入内核代码了，如果了解了用户程序如何切换到内核程序后，lab2的两个实验其实都不难。所以在开始实验之前，我们要去阅读一些实验给定的参考资料，了解**前置知识**并且**读懂题目**（这个真的很重要！）
前置知识：
- 内核程序的入口函数
- exec

在这里我先记录下我的理解，如有不对的地方，欢迎大家批评指正哦。在make qemu后，会通过user/usys.pl生成一个文件usys.S。这个脚本会根据每一个entry生成对应的用户程序转换到内核程序入口汇编代码:
```
//这是user/usys.pl中entry("fork")对应的汇编的代码
.global fork
fork:
 li a7, SYS_fork // 将SYS_fork传入寄存器a7中
 ecall // 发送ecall信号，
 ret
```
在用户程序中如果调用了fork()系统调用，就会触发系统执行上面这段汇编代码。而ecall对应的入口函数就是kernel/syscall.c。

# 二、trace

在了解了用户程序如何转入内核程序后，我们再来看下第一个实验究竟要做什么。这个实验需要我们实现的是在调用```trace mask cmd argv```命令后，跟踪执行```cmd argv```后，执行的```mask```对应的系统调用返回的参数。这个mask对应的是```1<<SYS_xx```。SYS_xx被声明在kernel/syscall.h中。换句话说，如果在执行```cmd argv```过程中，调用了SYS_xx编号对应的系统调用，内核就要输出```"<pid>: syscall <SYS_xx对应的系统调用函数名> -> <调用的系统调用函数返回的参数>"```。举个例子，```trace 32 grep hello README``` 这个命令中， ```1<<SYS_xx``` 就是 32， 即```SYS_xx = 5```，在kernel/syscall.h中可以找到5对应的是SYS_read。而在kernek/syscall.c中定义了函数指针数组syscalls,其中SYS_read对应的是系统调用read的函数指针。若在执行```grep hello README```后，调用了read系统调用，就应该打印```"<pid>: syscall read -> read()返回的参数"```

有了前面铺垫的知识后，就可以开始实现了。下面是我的实现过程，供大家参考。
## 1. 用户程序
- 在user/user.h中声明int trace(int) 
- 在user/usys.pl中添加entry("trace") 

## 2. 内核程序

因为只有执行了trace mask cmd argv命令的进程调用mask对应的系统调用时才要打印系统调用信息，因此我们需要做以下几步
- 在proc结构体里添加trace_mask成员
- 在kernel/sysproc.c中添加sys_trace()函数（参考kernel/sysproc.c其他函数的写法）

```c
uint64
sys_trace(void){
  int mask;
  if(argint(0,&mask) < 0){ // argint获取用户空间参数
    return -1;
  }
  myproc()->trace_mask = mask; //在调用了trace的系统调用的进程中记录用户传来的mask，这也是我们要跟踪的系统调用mask, 注意myproc()->trace_mask == 1 << SYS_xx
  return 0;
} 
```
- 在kernek/proc.c中的fork函数添加复制父进程trace_mask的代码```np->trace_mask = p->trace_mask;```，以及在kernel/proc.c中的freeproc函数中添加```p->trace_mask = 0;```
- 在kernel/syscall.c中的main函数中添加系统调用函数名数组以及修改syscall函数

```c
static char * syscall_names[] = {
[SYS_fork]    "fork",
[SYS_exit]    "exit",
[SYS_wait]    "wait",
[SYS_pipe]    "pipe",
[SYS_read]    "read",
[SYS_kill]    "kill",
[SYS_exec]    "exec",
[SYS_fstat]   "fstat",
[SYS_chdir]   "chdir",
[SYS_dup]     "dup",
[SYS_getpid]  "getpid",
[SYS_sbrk]    "sbrk",
[SYS_sleep]   "sleep",
[SYS_uptime]  "uptime",
[SYS_open]    "open",
[SYS_write]   "write",
[SYS_mknod]   "mknod",
[SYS_unlink]  "unlink",
[SYS_link]    "link",
[SYS_mkdir]   "mkdir",
[SYS_close]   "close",
[SYS_trace]   "trace",
};

void
syscall(void)
{
  int num;
  struct proc *p = myproc();
  num = p->trapframe->a7; // 存储了系统调用函数号
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num](); //p->trapframe->a0 存储了syscalls[num]的返回参数
  
    if (p->trace_mask & (1 << num)) // p->trace_mask == 1 << SYS_xx，  SYS_xx = num
    {
      printf("%d: syscall %s -> %d\n",p->pid, syscall_names[num], p->trapframe->a0);
    }
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
```
# 三、sysinfo
这题的需求是创建一个系统调用函数sysinfo,需要当前进程数以及剩余空间字节数。

1. 首先添加系统调用的步骤是：
    - 在user/user.h中声明，系统调用名sysinfo
    - 在user/usys.pl中添加entry("sysinfo")
    - 在kernel/syscall.h中定义SYS_sysinfo
    在usys.pl中可以看到entry汇编语言的模板是

    ```
    print "#include \"kernel/syscall.h\"\n";
    
    sub entry {
        my $name = shift;
        print ".global $name\n";
        print "${name}:\n";
        print " li a7, SYS_${name}\n";
        print " ecall\n";
        print " ret\n";
    }
    ```
    可以看到entry中的字符串是和SYS_xxx中的xxx对应的，所以我们应该在syscall.h中声明这个系统调用号且在syscall.c中也要按照格式引用。
    
    - 在kernel/syscall.c中，用extern引用sys_sysinfo系统调用，并且在系统调用函数指针数组中添加一项```[SYS_sysinfo] sys_sysinfo```，以及上一题声明的系统调用名数组中添加"sysinfo"
    - 在kernel/sysproc.c中实现sys_sysinfo函数(可以在kernel/sysproc.c中看到其他的系统调用函数都是在这个文件中实现，且函数格式都是sys_xxx)
2. 实现系统调用sys_sysinfo
```
int sys_sysinfo(void){
    struct sysinfo info;
    struct proc *p = myproc();
    uint64 adrr; // 需要拷贝info的用户空间目标地址   
    
    info.nproc = getProcNum();
    info.freemem = getFreeMemoryNum();
    
    if(argaddr(0,&adrr) < 0 || copyout(p->pagetable,adrr,(char*)&info,sizeof(info)) < 0){ //获取用户空间参数逻辑地址
        return -1;
    }
    return 0;
}
```
3. C语言的知识点补充
在syscall.c写接口的过程中一直很奇怪syscall是怎么索引到在外部文件定义的函数。其实这关系到C语言的一个关键字extern。默认情况下，一个函数定义在一个文件里，这个函数是所有文件都可见的，但是只有用extern关键字声明引用这个函数的文件里才能使用这个函数。


# 四、总结

在这个实验中，我们已经相对了解用户调用系统调用转入内核入口函数、内核程序如何从用户程序中读、写数据。
- 具体地syscall.h中会声明所有系统调用号，usys.pl会为每一个entry生成对应的汇编代码，并且在syscall.c中中会用extern声明引用已经在外部文件定义过的系统调用函数，通过myproc()->trapframe->a7声明系统调用号，在系统调用函数指针数组syscalls索引对应的函数指针。
- argint可以获取用户传递的参数；argaddr获取当前进程用户程序传递的地址；copyout可以向用户空间地址写入内核中程序输出的数据。


