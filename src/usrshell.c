#include "string.h"
#include "syscalls.h"
#include "kerneltypes.h"
#include "user-io.h"
#include "rtc.h"
#include "ascii.h"

void print_directory( char *d, int length )
{
  while(length>0) {
    printf("%s\n",d);
    int len = strlen(d)+1;
    d += len;
    length -= len;
  }
}

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
      int pid = fork();
      if (pid != 0) {
        printf("started process %d\n", pid);
      } else {
        exec(pch, argv, 2);
      }
    }
    else
      printf("start: missing argument\n");
  }
  else if (pch && !strcmp(pch, "run"))
  {
    pch = strtok(0, " ");
    if (pch) {
      const char* argv[] = {pch, "run"};
      int pid = fork();
      if (pid != 0) {
        printf("started process %d\n", pid);
        struct process_info info;
        while (process_wait(&info, 5000)) {}
        printf("process %d exited with status %d\n", info.pid, info.exitcode);
        process_reap(info.pid);
      } else {
        exec(pch, argv, 2);
      }
    }
    else
      printf("run: missing argument\n");
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
  else if (pch && !strcmp(pch, "mount"))
  {
    pch = strtok(0, " ");
    int unit;
    if (!(pch && str2int(pch, &unit))) {
      printf("Incorrect arguments, usage: mount <unit_no> <fs_type> <ns_name>\n");
      return 1;
    }
    char *fs_type = strtok(0, " ");
    char *ns_name = strtok(0, " ");
    mount(unit, fs_type, ns_name);
  }
  else if (pch && !strcmp(pch, "list"))
  {
    char buffer[1024];
    int length = readdir(".", buffer, 1024);
    print_directory(buffer, length);
  }
  else if (pch && !strcmp(pch, "chdir"))
  {
    char *path = strtok(0, " ");
    if (!path) {
      printf("Incorrect arguments, usage: chdir <path>\n");
      return 1;
    }
    chdir(path);
  }
  else if (pch && !strcmp(pch, "ns_change"))
  {
    char *ns = strtok(0, " ");
    if (!ns) {
      printf("Incorrect arguments, usage: chdir <path>\n");
      return 1;
    }
    ns_change(ns);
  }
  else if (pch && !strcmp(pch, "help"))
  {
    printf(
      "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
      "Commands:",
      "echo <text>",
      "run <path>",
      "mount <unit_no> <fs_type> <ns_name>",
      "chdir <path>",
      "ns_change <ns_name>",
      "list",
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
