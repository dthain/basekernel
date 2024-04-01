#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/errno.h"
// start-up program

int main(int argc, char const **argv[])
{
    const char *exec = "bin/process1.exe";
    argv[0] = exec;
    int pfd = syscall_open_file(KNO_STDDIR, exec, 0, 0);
    if (pfd >= 0)
    {
        printf("found %s\n", exec);
        int pid = syscall_process_run(pfd, argc,
                                      *argv);
        if (pid > 0)
        {
            printf("STARTED %d\n", pid);
        }
        else
        {
            printf("couldn't run %s: %s\n",
                   argv[0], strerror(pid));
        }
        syscall_object_close(pfd);
    }
    else
    {
        printf("couldn't find %s: %s\n",
               argv[0], strerror(pfd));
    }
    return 0;
}

