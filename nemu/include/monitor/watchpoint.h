#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  uint32_t NO;
  char msg[100];
  uint32_t val; // value of watch
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

/* TODO: if necessary, try to implement these functions freely in watchpoint.c
 * you can add other functions by yourself if necessary
 */
void new_wp(char *, int );
void free_wp(int);
void print_wp();
void delete_all_wp();

bool check_all_wp();

#endif
