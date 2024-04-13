#include "library/syscalls.h"
#include "library/stdio.h"

// invoked in test programs
void runForSeconds(int seconds)
{
    unsigned int startTime; // seconds
    syscall_system_time(&startTime);
    unsigned int timeElapsed;
    do
    {
        syscall_system_time(&timeElapsed);
        timeElapsed -= startTime;
    } while (timeElapsed < seconds);
}

int main(int argc, char const *argv[])
{
    // struct process_info info;
    // int pid = syscall_process_fork();
    // if (pid == 0)
    // {
    //     runForSeconds(1);
    //     printf("process4 done\n");
    //     return 0;
    // }
    // else
    // {
    //     printf("process 4   pid: %d\n", pid);
    //     syscall_process_wait(&info, -1);
    // }
    runForSeconds(1);
    printf("process4 with pid: %d done\n", syscall_process_self());
    return 0;
}
