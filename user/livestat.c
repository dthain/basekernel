#include "library/malloc.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

/**
 * Livestat is a command line utility to display a graph of operating system
 * statistics in realtime
 */

#define POINTS 60
#define ARG_MAX 16
#define ARG_SIZE 32

typedef enum STAT_LIVE {
  DRIVER_LIVE,
  BCACHE_LIVE,
  SYSTEM_LIVE,
  PROCESS_LIVE
} STAT_LIVE;

struct stat_args {
  /* General */
  STAT_LIVE stat_type;
  char * stat_name;
  void * statistics;
  /* Process */
  char * pid_s;
  int pid;
  int syscall_index;
  /* Driver */
  char * driver_name;
  /* System */
  int device_unit;
};

void help();
void stat_live_2_str(STAT_LIVE stat_l, char * str);
void create_graph(STAT_LIVE stat_type, char * stat_name, char * stat_arg, int window_width, int window_height, int plot_width, int plot_height, int thickness, int char_offset);
int  extract_statistic(struct stat_args * args);
void plot_bars(int * most_recent_vals, int max, int window_width, int window_height, int plot_width, int plot_height, int thickness, int char_offset);
void run_stats(struct stat_args * args);

int main(int argc, const char * argv[]) {
  /* Setup program parameters */
  int current_arg     = 1;
  struct stat_args args = { 0 };

  /* Parse command line options */
  if (argc < 2)  {
    help(1); return 1;
  }

  while(current_arg < argc) {
    if (!strcmp(argv[current_arg], "-h") || !strcmp(argv[current_arg], "--help")) {
      help(); return 0;
    }
    else if (!strcmp(argv[current_arg], "-b")) {
      args.statistics = malloc(sizeof(struct bcache_stats));
      args.stat_type = BCACHE_LIVE;
    }
    else if (!strcmp(argv[current_arg], "-dr")) {
      args.statistics = malloc(sizeof(struct device_driver_stats));
      args.stat_type = DRIVER_LIVE;
      args.driver_name = strdup(argv[++current_arg]);
    }
    else if (!strcmp(argv[current_arg], "-sys")) {
      args.statistics = malloc(sizeof(struct system_stats));
      args.stat_type = SYSTEM_LIVE;
    }
    else if (!strcmp(argv[current_arg], "-p")) {
      args.statistics = malloc(sizeof(struct process_stats));
      args.stat_type = PROCESS_LIVE;
      args.pid_s = strdup(argv[++current_arg]);
      str2int(args.pid_s, &(args.pid));
    }
    else if (!strcmp(argv[current_arg], "-s")) {
      args.stat_name = strdup(argv[++current_arg]);
    }
    else if (!strcmp(argv[current_arg], "-sc")) {
      str2int(argv[++current_arg], &(args.syscall_index));
    }
    else if (!strcmp(argv[current_arg], "-du")) {
      str2int(argv[++current_arg], &(args.device_unit));
    }
    else {
      help(); return 1;
    }
    ++current_arg;
  }

  if (!args.stat_name) {
    help(); return 1;
  }

  /* Start tracking stats */
  run_stats(&args);

  return 0;
}

/** HELPER FUNCTIONS **/
/* Create graph and continuously update it */
void run_stats(struct stat_args * args) {
  /* Initialize variables for graph and stats collection */
  int window_width  = 270;
  int window_height = 270;
  int plot_width    = 180;
  int plot_height   = 180;
  int thickness     = 2;
  int char_offset   = 8;

  /* Generate the graph with correct labels */
  if (args->stat_type == BCACHE_LIVE) {
    create_graph(args->stat_type, args->stat_name, 0, window_width, window_height, plot_width, plot_height, thickness, char_offset);
  } else if (args->stat_type == PROCESS_LIVE) {
    create_graph(args->stat_type, args->stat_name, args->pid_s, window_width, window_height, plot_width, plot_height, thickness, char_offset);
  } else if (args->stat_type == DRIVER_LIVE) {
    create_graph(args->stat_type, args->stat_name, args->driver_name, window_width, window_height, plot_width, plot_height, thickness, char_offset);
  } else if (args->stat_type == SYSTEM_LIVE) {
    create_graph(args->stat_type, args->stat_name, 0, window_width, window_height, plot_width, plot_height, thickness, char_offset);
  }

  /* Start tracking stats */
  int i, new_val, max;
  int most_recent_vals[POINTS];
  int last_value = 0;
  int first = 1;
  char quit = 0;

  for (size_t i = 0; i < POINTS; i++)
    most_recent_vals[i] = -1;

  while(1) {
    /* Check to exit */
    syscall_object_read_nonblock(0, &quit, 1);
    if (quit == 'q') break;

    /* Get the correct stats */
    if (args->stat_type  == BCACHE_LIVE) {
      syscall_bcache_stats(args->statistics);
    } else if (args->stat_type  == PROCESS_LIVE) {
      syscall_process_stats(args->statistics, args->pid);
    } else if (args->stat_type == DRIVER_LIVE) {
      syscall_device_driver_stats(args->driver_name, args->statistics);
    } else if (args->stat_type  == SYSTEM_LIVE) {
      syscall_system_stats(args->statistics);
    }

    /* Grab the specified statistic of interest */
    new_val = extract_statistic(args);

    /* Skip first value because it is just a baseline */
    if (first) {
      last_value = new_val;
      first = 0;
      goto sleep;
    }

    /* Update points */
    for (i = 1; i < POINTS; i++)
      most_recent_vals[i-1] = most_recent_vals[i];
    most_recent_vals[POINTS-1] = new_val - last_value;
    last_value = new_val;

    /* Set max to greatest point or 1 */
    max = 1;
    for (i = 0; i < POINTS; i++) {
      if (most_recent_vals[i] > max) max = most_recent_vals[i];
    }

    /* Replot points */
    plot_bars(most_recent_vals, max, window_width, window_height, plot_width, plot_height, thickness, char_offset);

    /* Sleep for 6 seconds and then continue */
    sleep:
    syscall_process_sleep(6000);
  }
  draw_color(255, 255, 255);
  draw_flush();
}

/* Return one stat from statistics structure */
int extract_statistic(struct stat_args * args) {

  if (args->stat_type == BCACHE_LIVE) {
    if (!strcmp(args->stat_name, "read_hits")) {
      return ((struct bcache_stats *)args->statistics)->read_hits;
    } else if (!strcmp(args->stat_name, "read_misses")) {
      return ((struct bcache_stats *)args->statistics)->read_misses;
    } else if (!strcmp(args->stat_name, "write_hits")) {
      return ((struct bcache_stats *)args->statistics)->write_hits;
    } else if (!strcmp(args->stat_name, "write_misses")) {
      return ((struct bcache_stats *)args->statistics)->write_misses;
    } else if (!strcmp(args->stat_name, "writebacks")) {
      return ((struct bcache_stats *)args->statistics)->writebacks;
    }
  }
  else if (args->stat_type == PROCESS_LIVE) {
    if (!strcmp(args->stat_name, "blocks_read")) {
      return ((struct process_stats *)args->statistics)->blocks_read;
    } else if (!strcmp(args->stat_name, "blocks_written")) {
      return ((struct process_stats *)args->statistics)->blocks_written;
    } else if (!strcmp(args->stat_name, "bytes_read")) {
      return ((struct process_stats *)args->statistics)->bytes_read;
    } else if (!strcmp(args->stat_name, "bytes_written")) {
      return ((struct process_stats *)args->statistics)->bytes_written;
    } else if (!strcmp(args->stat_name, "syscall_count")) {
      return ((struct process_stats *)args->statistics)->syscall_count[args->syscall_index];
    }
  }
  else if (args->stat_type == DRIVER_LIVE) {
      if (!strcmp(args->stat_name, "blocks_read")) {
        return ((struct device_driver_stats *)args->statistics)->blocks_read;
      } else if (!strcmp(args->stat_name, "blocks_written")) {
        return ((struct device_driver_stats *)args->statistics)->blocks_written;
      }
  }
  else if (args->stat_type == SYSTEM_LIVE) {
    if (!strcmp(args->stat_name, "time")) {
      return ((struct system_stats *)args->statistics)->time;
    } else if (!strcmp(args->stat_name, "blocks_read")) {
      return ((struct system_stats *)args->statistics)->blocks_read[args->device_unit];
    } else if (!strcmp(args->stat_name, "blocks_written")) {
      return ((struct system_stats *)args->statistics)->blocks_written[args->device_unit];
    }
  }

  return -1;
}

/* Create the graph template for the display */
void create_graph(STAT_LIVE stat_type, char * stat_name, char * stat_arg, int window_width, int window_height, int plot_width, int plot_height, int thickness, int char_offset) {
  /* Initialize Parameters */
  int i;
  int x_offset      = (window_width - plot_width) / 2;
  int y_offset      = (window_height - plot_height) / 2;
  char title[100] = "";

  stat_live_2_str(stat_type, title);
  if (stat_arg != 0) {
    strcat(title, ": ");
    strcat(title, stat_arg);
  }

  /* Draw border around window */
  draw_window(KNO_STDWIN);
  draw_clear(0, 0, window_width, window_height);
  draw_rect(0, 0, window_width, thickness);
  draw_rect(0, 0, thickness, window_height);
  draw_rect(window_width - thickness, 0, thickness, window_height);
  draw_rect(0, window_height - thickness, window_width, thickness);

  /* Draw X, Y axis in center of window */
  draw_rect(x_offset, y_offset+plot_height, plot_width, thickness);
  draw_rect(x_offset, y_offset, thickness, plot_height);

  /* Draw title, x_axis label, y_axis label */
  draw_string(window_width/2 - 70, 8, title);
  draw_string(window_width/2 - 34, window_height - 16, "Time (6s)");

  /* Setup Y axis */
  char y[2] = { 0 };
  for (i = 0; i < strlen(stat_name); i++) {
      y[0] = stat_name[i];
      draw_string(10, window_height/2 - 20 + i*char_offset, y);
      draw_flush();
  }

  /* For tick marks on x axis -- always keep the most recent minute - 10 ticks */
  int x_tick_width = plot_width/10;
  for (i = 0; i < 10+1; i++) {
    draw_rect(x_offset+i*x_tick_width, y_offset+plot_height, thickness, 6);
  }

  /* For tick marks on y axis -- max will be dynamically set */
  int y_tick_width = plot_height/10;
  for (i = 0; i < 10+1; i++) {
    draw_rect(x_offset - 4, y_offset + plot_height - i * y_tick_width, 6, thickness);
  }

  draw_flush();
}

/* Plot all of the points on the graph and print the max */
void plot_bars(int most_recent_vals[POINTS], int max, int window_width, int window_height, int plot_width, int plot_height, int thickness, int char_offset) {
  /* Clear the graph */
  int x_offset      = (window_width - plot_width) / 2;
  int y_offset      = (window_height - plot_height) / 2;
  draw_clear(x_offset, y_offset,  plot_width+thickness, plot_height);

  /* Redraw the axis */
  draw_rect(x_offset, y_offset+plot_height, plot_width, thickness);
  draw_rect(x_offset, y_offset, thickness, plot_height);

  /* Create string of the max */
  char max_str[32];
  uint_to_string((uint32_t) max, max_str);
  draw_string(x_offset - 26, y_offset - 10, max_str);
  draw_flush();

  /* Plot the points */
  int i, current_point[2] = { 0 };

  /* Variables to plot value in correct location on the graph */
  float x_step = (float)plot_width/POINTS;
  float y_step = (float)plot_height/max;

  draw_color(0, 255, 0);

  for (i = 0; i < POINTS; i++) {
    if (most_recent_vals[i] == -1)
      continue;

    current_point[0] = x_offset + (int)(x_step*(i+1));
    current_point[1] = y_offset + plot_height - (int)(y_step*most_recent_vals[i]);
    draw_rect(current_point[0], current_point[1], thickness, (int)(y_step*most_recent_vals[i]));
  }
  draw_flush();
  draw_color(255, 255, 255);
}

/* Convert stat type to string */
void stat_live_2_str(STAT_LIVE stat_l, char * str) {

  if (stat_l == BCACHE_LIVE) {
    strcpy(str, "Bcache");
  }
  else if (stat_l == PROCESS_LIVE) {
    strcpy(str, "Process");
  }
  else if (stat_l == DRIVER_LIVE) {
    strcpy(str, "Driver");
  }
  else if (stat_l == SYSTEM_LIVE) {
    strcpy(str, "System");
  }
}

/* Help message */
void help() {
  printf("\nusage\n\n");
  printf("statslive.exe\n");
  printf("                -h | --help          # display help\n");
  printf("                -b                   # buffer cache stats\n");
  printf("                -dr  <DRIVER_NAME>   # driver stats\n");
  printf("                -sys <BLOCK>         # system stats\n");
  printf("                -p   <PID>           # process stats\n");
  printf("                -sc  <SYSCALL>       # syscall number\n");
  printf("                -s   <STAT_NAME>     # name of statistic\n");

  printf("\nBuffercache STAT_NAME options:\n");
  printf("    read_hits\n");
  printf("    read_misses\n");
  printf("    write_hits\n");
  printf("    write_misses\n");
  printf("    writebacks\n\n");

  printf("\nProcess STAT_NAME options:\n");
  printf("    blocks_read\n");
  printf("    blocks_written\n");
  printf("    bytes_read\n");
  printf("    bytes_written\n");
  printf("    syscall_count\n\n");

  printf("\nDriver STAT_NAME options:\n");
  printf("    blocks_read\n");
  printf("    blocks_written\n\n");

  printf("\nSystem STAT_NAME options:\n");
  printf("    time\n");
  printf("    blocks_read\n");
  printf("    blocks_written\n\n");

  //TODO memory utilizations (page), kernel malloc

}
