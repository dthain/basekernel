#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

#include "kernel/stats.h"

/* Object Stats
 * This is a simple program to test the object stats system call
 * Stat collection for the following objects are collected:
 *   Object
 *
 */


int main()
{
  /* Setup */
  int fd, n, pid;
  char buffer[1000];
  printf("\n*****************\n");
  printf("Object Stats Test\n");
  printf("*****************\n\n");

  /* Object stats collection */
  printf("Testing Stats Collection: Object\n");
  fd = syscall_open_file("/bin/objectstats.exe", 0, 0);
	while((n = syscall_object_read(fd, buffer, 100)) > 0) {
		buffer[n] = 0;
		flush();
	}

  struct object_stats obj_stats;
  syscall_object_stats(fd, &obj_stats, OBJECT_TYPE);
  printf("reads:         %d\n", obj_stats.reads);
  printf("writes:        %d\n", obj_stats.writes);
  printf("bytes_read:    %d\n", obj_stats.bytes_read);
  printf("bytes_written: %d\n\n", obj_stats.bytes_written);
  syscall_object_close(fd);

  /* Pipe stats */
  printf("Testing Stats Collection: Pipes\n");
  fd = syscall_open_pipe();
  pid = syscall_process_fork();

	if(pid) {
    char * buf = "Hello!";
		syscall_object_write(fd, buf, strlen(buf));
		syscall_process_sleep(1000);
	} else {
		while(!(n = syscall_object_read(fd, buffer, 10))) {
			syscall_process_yield();
		}
    return 0;
	}

  struct pipe_stats pipe_stats;
  syscall_object_stats(fd, &pipe_stats, PIPE_TYPE);
  printf("reads:         %d\n", pipe_stats.reads);
  printf("writes:        %d\n\n", pipe_stats.writes);
  syscall_object_close(fd);

  /* File stats */
  printf("Testing Stats Collection: Directory Entries\n");
  fd = syscall_open_file("/bin/objectstats.exe", 0, 0);

  while((n = syscall_object_read(fd, buffer, 1000)) > 0) {
		buffer[n] = 0;
		flush();
	}

  struct dirent_stats dirent_stats;
  syscall_object_stats(fd, &dirent_stats, FILE_TYPE);
  printf("reads:         %d\n", dirent_stats.reads);
  printf("writes:        %d\n\n", dirent_stats.writes);
  syscall_object_close(fd);

  /* Console stats */
  printf("Testing Stats Collection: Console\n");
  struct console_stats console_stats;
  syscall_object_stats(KNO_STDOUT, &console_stats, CONSOLE_TYPE);
  printf("writes:         %d\n", console_stats.writes);
  printf("bytes_written:  %d\n\n", console_stats.bytes_written);

  /* Graphics stats */
  printf("Testing Stats Collection: Graphics\n");
  draw_window(KNO_STDWIN);
  draw_line(0,0,4,4);
  draw_flush();
  struct graphics_stats graphics_stats;
  syscall_object_stats(KNO_STDOUT, &graphics_stats, GRAPHICS_TYPE);
  printf("writes:         %d\n\n", graphics_stats.writes);

  /* Device stats */
  printf("Testing Stats Collection: Device\n");
  struct device_stats device_stats;
  fd = syscall_open_file("/bin/objectstats.exe", 0, 0);
  syscall_object_stats(fd, &device_stats, DEVICE_TYPE);
  printf("reads:         %d\n", device_stats.reads);
  printf("writes:        %d\n\n", device_stats.writes);
  syscall_object_close(fd);

  return 0;
}
