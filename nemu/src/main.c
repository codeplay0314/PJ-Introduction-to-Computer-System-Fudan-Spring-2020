int init_monitor(int, char *[]);
void ui_mainloop(int);
#include <stdio.h>

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);
  printf("---%s----", argv[0]);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}
