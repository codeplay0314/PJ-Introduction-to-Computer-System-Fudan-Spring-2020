#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  bool is_free, changed;
  char msg[100];
  int val, cval;
}WP;

WP *new_wp(char *, int );
void free_wp(int);
void print_wp();
void delete_all_wp();
int check_wp();

#endif
