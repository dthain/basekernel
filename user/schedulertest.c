#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/errno.h"
// #include "list.h"

int create_process(const char *exec, int priority)
{
    int pfd = syscall_open_file(KNO_STDDIR, exec, 0, 0);
    
    if (pfd >= 0)
    {
        // const char *args[] = {exec, NULL};
        int pid = syscall_process_prun(pfd, 0, &exec, priority);
        if (pid > 0)
        {
            // printf("STARTED pid: %d\n", pid);
            printf("created %s with priority %d\n", exec, priority);
        }
        else
        {
            printf("couldn't run %s: %s\n", exec, strerror(pid));
        }
        syscall_object_close(pfd);
        
    }
    else
    {
        printf("couldn't find %s: %s\n", exec, strerror(pfd));
    }
    return 0;
}

int main(int argc, char const *argv[])
{
    const char *procs[] = {"bin/process1.exe", "bin/process2.exe", "bin/process3.exe", "bin/process4.exe", "bin/process5.exe"};
    int priorities[] = {9, 7, 2, 1, 5};
    for (int i = 0; i < 2; i++)
    {
        create_process(procs[i], priorities[i]);
    }
    // When this works we r done q1 just need to trivially make other procs
    // syscall_run_all();    
    return 0;
}