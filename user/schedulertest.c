#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/errno.h"

int run(const char *exec)
{
    int pfd = syscall_open_file(KNO_STDDIR, exec, 0, 0);
    if (pfd >= 0)
    {
        printf("running %s\n", exec);
        // const char *args[] = {exec, NULL};
        int pid = syscall_process_run(pfd, 0, &exec);
        if (pid > 0)
        {
            // printf("STARTED pid: %d\n", pid);
            printf("waiting for %s to finish\n", exec);
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
    const char *args[] = {"bin/process1.exe", "bin/process2.exe"};

    for (int i = 0; i < 2; i++)
    {
        run(args[i]);
    }
    return 0;
}