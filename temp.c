#include <stdio.h>
#include <cutils/sockets.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <sys/resource.h>

void socket_test();
void coredump_test();
static void handle_signal();
static void SIGCHLD_handler(int k);
void register_epoll_handler(int fd, void (*fn)());

static int signal_write_fd = -1;
static int signal_read_fd = -1;
static int epoll_fd = -1;
static int quit = 1; 
int main(int argc, char ** argv )
{
   int i = 1;
   struct rlimit rl;
   rl.rlim_cur = RLIM_INFINITY;
   rl.rlim_max= RLIM_INFINITY;
   if(setrlimit(RLIMIT_CORE, &rl)== -1){
        printf("set RLIMIT failed\n");      
    }else
        printf("set RLIMIT successful \n");
   
    for(i; i< argc; i++)
    {
        if(!strcmp(argv[i], "-s"))
        {
           socket_test();
           return 1; 
        }else if(!strcmp(argv[i], "-f")){
           fork_test();
           return 1;
        }else if(!strcmp(argv[i], "-c")){
           coredump_test();
           return 1;
        }else if(!strcmp(argv[i], "-n"))
            fork_nu_test();
	    return 1;	
     } 

    //printf("I'm in main %d\n", getpid());
    return 1;
}
void fork_test()
{
   
    int count = 1; 
    printf("before fork, PID is %d\n", getpid());
    int result = fork();
    if(result < 0)
       printf("fork error"); 
    else if(result==0)
        {
        count ++ ;
        printf("in the child process,   PID is %d  count is %d\n", getpid(), count);
        }
    else if(result > 0)
        {
       count ++ ;
       printf("in the parent  process PID is %d result is %d count is %d \n", getpid(),result, count);
        }

}
void fork_nu_test()
{
   
    //int count = 1; 
    fork();
    fork();
    fork();
    fork();
    sleep(10);
    printf("fork, PPID is %d      PID is %d\n", getppid(), getpid());
    sleep(3);
    exit(1);

}
void socket_test()
{
    int s[2];
    char buf[32];
    int signal;
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    
    if(socketpair(AF_UNIX, SOCK_STREAM,0, s)==-1){
     printf("socket pair failed\n");
     exit(1);
    }
   
    signal_write_fd = s[0];
    signal_read_fd = s[1];

    // Write to signal_write_fd if we catch SIGCHLD.
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = SIGCHLD_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;//SA_NOCLDSTOP;
    sigaction(SIGABRT, &act, 0);
    sigaction(SIGSEGV, &act, 0);
    //reap_any_outstanding_children();
   // register_epoll_handler(signal_read_fd, handle_signal);
    if(fork()==0)
    {
    printf("waiting for read \n");
    read(signal_read_fd, &signal, sizeof(signal));
    printf("read data from socket %d, %d\n", signal_read_fd, signal);
    printf("process PPID = %d  PID = %d\n", getppid(), getpid()); 
    close(s[0]);
    close(s[1]);
    }else{
        close(s[0]);
        close(s[1]);
    }
   /* while(quit)
    {
	sleep(3000);
    }*/
    exit(1);

}

static void handle_signal() {
    // Clear outstanding requests.
    char buf[32];
    read(signal_read_fd, buf, sizeof(buf));
    
    printf("handle_signal,  read data from socket %d, %s\n", signal_read_fd, buf);
    quit =0; 
    //reap_any_outstanding_children();
}

static void SIGCHLD_handler(int k) {
	
    printf("INIT#, child process dead k= %d\n", k);
    if (TEMP_FAILURE_RETRY(write(signal_write_fd, &k, sizeof(k))) == -1) {
        printf("write(signal_write_fd) failed: %s\n", strerror(errno));
    }
    
}

/*
void register_epoll_handler(int fd, void (*fn)()) {
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = reinterpret_cast<void*>(fn);
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        printf("epoll_ctl failed: %s\n", strerror(errno));
    }
}*/
void coredump_test()
{
   char *a = NULL ;
   *a = 0x323;
	
}
