#include "syscalls.h"
#include "user-io.h"
#include "string.h"

int main( const char *argv[], int argc )
{
    printf("%d: Running pipe test!\n", process_self());
    int w = pipe_open();
    set_blocking(w, 0);
    int x = fork();
    if (x) {
        printf("%d: Writing...\n", process_self());
        dup(w, KNO_STDOUT);
        printf("Testing!\n");
        sleep(1000);
    } else {
        printf("%d: Reading...\n", process_self());
        int r;
        char buf[10];
        while (!(r = read(w, buf, 10))) {
            yield();
        }
        printf("%d: I read %d chars from my brother\n", process_self(), r);
        printf("%d: They are (%s)\n", process_self(), buf);
    }
	return 0;
}
