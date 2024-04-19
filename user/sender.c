#include "library/syscalls.h"
#include "library/string.h"
#include "../kernel/string.h"
#include "../include/kernel/types.h"

int main() {
    printf("%d: Running named pipe test sender!\n", syscall_process_self());
    const char *fname = "bin/named_pipe";

    // message to send
    char buffer[] = "Hello World\n";
    int res = syscall_make_named_pipe(fname);
    printf("syscall_make_named_pipe: %d",res);
    // create the named pipe
    int fd = syscall_open_named_pipe(fname);
    if(fd >= 0){
        printf("%d: Named pipe successfully open.\n", syscall_process_self());
        printf("%d: Entering Sending process.\n", syscall_process_self());
        syscall_object_write(fd, buffer, strlen(buffer), 0); // write the message to the named pipe
        printf("%d: Message written to named pipe.\n", syscall_process_self());
        syscall_process_sleep(1000);
        return 1; 
        } 

    else {
        printf("%d: Error opening named pipe.\n", syscall_process_self());
        return 1;
    }

  return 1;
}