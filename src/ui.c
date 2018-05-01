#include "syscalls.h"
#include "user-io.h"
#include "string.h"


void forward(int i, int o) {
    char buf[1024];
    read(i, buf, 1023);
    write(o, buf, 1024);
}

int s = 100;

int main( const char *argv[], int argc )
{
    int w = pipe_open();
    int w2 = pipe_open();
    set_blocking(w, 0);
    set_blocking(w2, 0);
    int x = fork();
    if (x) {
        dup(w, KNO_STDOUT);
        for (;;) {
            printf(",%dx%dy\n", s, s);
            forward(w2, KNO_STDWIN);
            sleep(100);
            char c;
            if (read(KNO_STDIN, &c, 1) > 0) {
                if (c == '1') {
                    s = 400;
                }
            }
        }
    } else {
        dup(w2, KNO_STDWIN);
        const char *args[] = {"saver.exe"};
        exec("saver.exe", args, 1);
        printf("%d: ERROR!...\n", process_self());
    }
	return 0;
}
