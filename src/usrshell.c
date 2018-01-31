#include "string.h"
#include "syscalls.h"
#include "kerneltypes.h"
#include "user-io.h"
#include "rtc.h"
#include "ascii.h"

int process_command(char * line) {
  const char *pch = strtok(line, " ");
  if (pch && !strcmp(pch, "echo"))
  {
    pch = strtok(0, " ");
    if (pch) printf("%s\n", pch);
  }
  else if (pch && !strcmp(pch, "start"))
  {
    pch = strtok(0, " ");
    if (pch) {
            const char* argv[] = {pch, "start"};
      int pid = process_run(pch, argv, 2);
            printf("started process %d\n", pid);
      yield();
    }
    else
      printf("run: missing argument\n");
  }
  else if (pch && !strcmp(pch, "run"))
  {
    pch = strtok(0, " ");
    if (pch) {
            const char* argv[] = {pch, "run"};
      int pid = process_run(pch, argv, 2);
            printf("started process %d\n", pid);
            struct process_info info;
            if (!process_wait(&info, 5000)) {
                printf("process %d exited with status %d\n", info.pid, info.exitcode);
                process_reap(info.pid);

            } else {
                printf("run: timeout\n");
            }
    }
    else
      printf("run: missing argument");
  }
  else if (pch && !strcmp(pch, "passrun"))
  {
    pch = strtok(0, " ");
    if (pch) {
            const char* argv[] = {pch, "passrun"};
      int pid = process_run(pch, argv, 2);
            printf("started process %d\n", pid);
            struct process_info info;
            while (process_wait(&info, 5000)) {}
            printf("process %d exited with status %d\n", info.pid, info.exitcode);
            process_reap(info.pid);
    }
    else
      printf("run: missing argument");
  }
  else if (pch && !strcmp(pch, "reap"))
  {
    pch = strtok(0, " ");
        int pid;
    if (pch && str2int(pch, &pid)) {
        if (process_reap(pid)) {
                printf("reap failed!\n");
            } else {
                printf("processed reaped!\n");
            }
        }
    else
      printf("reap: expected process id number but got %s\n", pch);

  }
  else if (pch && !strcmp(pch, "kill"))
  {
    pch = strtok(0, " ");
        int pid;
    if (pch && str2int(pch, &pid)) {
        process_kill(pid);  
        }
    else
      printf("kill: expected process id number but got %s\n", pch);

  }
  else if (pch && !strcmp(pch, "wait"))
  {
    pch = strtok(0, " ");
    if (pch)
      printf("%s: unexpected argument\n", pch);
    else {
            struct process_info info;
            if (!process_wait(&info, 5000)) {
                printf("process %d exited with status %d\n", info.pid, info.exitcode);

            } else {
                printf("wait: timeout\n");
            }
        }

  }
  else if (pch && !strcmp(pch, "help"))
  {
    printf(
      "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
      "Commands:",
      "echo <text>",
      "run <path>",
      "passrun <path>",
      "start <path>",
            "kill <pid>",
            "reap <pid>",
            "wait",
      "help",
      "exit"
    );
  }
  else if (pch && !strcmp(pch, "exit"))
  {
    return -1;
  }
  else if (pch)
  {
    printf("%s: command not found\n", pch);
  }
  return 0;
}

int main(char ** argv, int argc) {
  printf("User shell ready:\n");
  char line[1024];
  char * pos = line;
  char c;
  printf("$ ");
  while (1) {
    flush();
    c = keyboard_read_char();
    if (pos == line && c == ASCII_BS)
      continue;
    printf_putchar(c);
    flush();
    if (c == ASCII_CR) {
      int res = process_command(line);
      if (res < 0) {
        break;
      }
      pos = line;
      printf("$ ");
    }
    else if (c == ASCII_BS) {
      pos--;
    }
    else {
      *pos = c;
      pos++;
    }
    *pos = 0;
  }
  return 0;
}
