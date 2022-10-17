#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

int main(int argc,char **argv)
{
        pid_t  pid;

        printf("parent process PID[%d] start running!!!\n",getpid());

        pid = fork();

        printf("PID: %d", pid);
        if(pid < 0)
        {
                printf("\n\n");
                printf("fork() create child process failure: %s\n",strerror(errno));
                return -1;
        }
        else if(pid == 0)
        {
                printf("\n");
                printf("child process PID[ %d ] start running,my parent PID is [ %d ]\n",getpid(),getppid());
                return 0;
        }
        else    //if(pid > 0)
        {
                printf("\n");
                sleep(2);
                printf("parent process PID[%d] continue running,and child process PID is [ %d ]\n",getpid(),pid);
                printf("\n");
                return 0;
        }
}
