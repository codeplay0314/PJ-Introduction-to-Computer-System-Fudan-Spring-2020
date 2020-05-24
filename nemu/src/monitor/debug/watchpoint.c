#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {}; 

// TODO: try to re-organize, you can abort head and free_ pointer while just use static int index
static WP *head, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i+1;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;
  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(char * msg, int val){
  return free_;
}

void free_wp(int no){
  return;
}

void print_wp(){
  printf("watchpoint: not yet implemented");
  return;
}

void delete_all_wp(){
  return;
}
