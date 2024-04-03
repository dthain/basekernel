#include "process.h"
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
        //can add logic here to check for other processes with higher priority`
        timeElapsed -= startTime;
    } while (timeElapsed < seconds);
    printf("waited %d seconds\n", timeElapsed);
}

int main(int argc, char const *argv[])
{
    struct process_info info;
    printf("process2\n");
    int pid = syscall_process_fork();
    if (pid == 0)
    {
        runForSeconds(5);
        return 0;
    }
    else
    {
        printf("pid: %d\n", pid);
        syscall_process_wait(&info, -1);
    }
    return 0;
}
