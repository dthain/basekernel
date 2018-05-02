#include "syscalls.h"
#include "user-io.h"
#include "string.h"

#define WINDOWS (2)

void forward(int i, int o) {
    char buf[1024] = {0};
    read(i, buf, 1023);
    write(o, buf, 1024);
}

int active = 0;
int cmd_pipes[WINDOWS] = {0};
int win_pipes[WINDOWS] = {0};
int side_panels[WINDOWS] = {0};

const char *BIG = ",500x500y\n";
const char *SMALL = ",100x100y\n";

int main( const char *argv[], int argc )
{
    int i;
    for (i = 0; i < WINDOWS; i++) {
        cmd_pipes[i] = pipe_open();
        win_pipes[i] = pipe_open();
        side_panels[i] = draw_create(KNO_STDWIN,500,200*i,100,100);
        set_blocking(cmd_pipes[i], 0);
        set_blocking(win_pipes[i], 0);
        if (i == active) {
            write(cmd_pipes[i], (void*)BIG, strlen(BIG));
        } else {
            write(cmd_pipes[i], (void*)SMALL, strlen(SMALL));
        }
    }
    set_blocking(KNO_STDIN, 0);
    int x = fork();
    if (x) {
        for (;;) {
            for (i = 0; i < WINDOWS; i++) {
                if (i == active) {
                    forward(win_pipes[i], KNO_STDWIN);
                } else {
                    forward(win_pipes[i], side_panels[i]);
                }
            }
            sleep(100);
            char c = 0;
            if (read(KNO_STDIN, &c, 1) > 0) {
                if (c == '1') {
                    write(cmd_pipes[active], (void*)SMALL, strlen(SMALL));
                    active = 0;
                    write(cmd_pipes[active], (void*)BIG, strlen(BIG));
                    draw_window(KNO_STDWIN);
                    draw_clear(0, 0, 600, 600);
                    draw_flush();
                } else if (c == '2') {
                    write(cmd_pipes[active], (void*)SMALL, strlen(SMALL));
                    active = 1;
                    write(cmd_pipes[active], (void*)BIG, strlen(BIG));
                    draw_window(KNO_STDWIN);
                    draw_clear(0, 0, 600, 600);
                    draw_flush();
                }
            }
        }
    } else {
        int x = fork();
        if (x) {
            dup(win_pipes[0], KNO_STDWIN);
            dup(cmd_pipes[0], 4);
            const char *args[] = {"saver.exe"};
            exec("saver.exe", args, 1);
            printf("%d: ERROR!...\n", process_self());
        } else {
            dup(win_pipes[1], KNO_STDWIN);
            dup(cmd_pipes[1], 4);
            const char *args[] = {"saver.exe"};
            exec("agclk.exe", args, 1);
            printf("%d: ERROR!...\n", process_self());
        }
    }
	return 0;
}
