#include "library/syscalls.h"
#include "library/string.h"
#include "../kernel/string.h"
#include "../include/kernel/types.h"

int main() {
    syscall_process_sleep(10);
    printf("%d: Running named pipe test receiver!\n", syscall_process_self());
    const char *fname = "bin/named_pipe";

    int fd = syscall_open_named_pipe(fname);
    if(fd >= 0){
        printf("%d: Named pipe successfully open.\n", syscall_process_self());

        // parent process: receiver
        char received_message[20];
        int r;

        printf("%d: Entering Reading process\n", syscall_process_self());
        while((r = syscall_object_read(fd, received_message, 20, -1))==0) {
            syscall_process_yield();
        }
            
        if(r > 0 ){
            received_message[r] = '\0';
            printf("%d: Message received: %s\n", syscall_process_self(), received_message);
        } else {
            printf("%d: Error reading the message %s\n", syscall_process_self(), r);
        }

        syscall_object_close(fd); 
        printf("%d: Named pipe closed.\n", syscall_process_self());
        printf("%d: Named pipe test finish.\n", syscall_process_self());
        }

    else {
        printf("%d: Error opening named pipe.\n", syscall_process_self());
        return 1;
    }

  return 1;
}