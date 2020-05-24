#include "nemu.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};

// TODO: try to re-organize, you can abort head and free_ pointer while just use static int index
static WP *head, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i+1;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].is_free = 1;
    wp_pool[i].changed = wp_pool[i].cval = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP *new_wp(char *msg, int val) {
  WP* newfree;
  newfree = (WP*)malloc(sizeof(WP));
  newfree = free_->next;

  strcpy(free_->msg, msg);
  free_->val = val; free_->next = head;
  free_->is_free = 0;free_->changed = 0;free_->cval = 0;
  head = free_;free_ = newfree;

  printf("Successfully create watchpoint %s.\n", msg);
  return free_;
}

void free_wp(int no) {
  WP* pre_wp;
  pre_wp = NULL;

  for (WP* now_wp = head;now_wp !=  NULL;now_wp = now_wp->next) {
    if (now_wp->NO == no) {

      if (pre_wp == NULL) {
        head = now_wp->next;
      }
      else {
        pre_wp->next = now_wp->next;
      }
      now_wp->is_free = 1;
      now_wp->next = NULL;
      Log("free watchpoint succeed.");
      break;
    }
    pre_wp = now_wp;
  }

  return;
}

void print_wp() {
  for (WP* now_wp = head;now_wp !=  NULL;now_wp = now_wp->next) {
    printf("watchpoint %d: msg:%s val:%d\n", now_wp->NO, now_wp->msg, now_wp->val);
  }
  return;
}

void delete_all_wp() {
  for (WP* now_wp = head;now_wp !=  NULL;now_wp = now_wp->next) {
    free_wp(now_wp->NO);
  }
  return;
}

int check_wp() {
  int flag = 0;
  for (WP* now_wp = head;now_wp !=  NULL;now_wp = now_wp->next) {
    bool *success;
    success = (bool *)malloc(sizeof(bool));
    uint32_t now_val = expr(now_wp->msg, success);
    if (*success) {
      if (now_wp->val != now_val) {
        now_wp->changed = 1;now_wp->cval = now_val;
        flag = 1;
      }
    }else {Log("strange expr in check_wp.");}
  }
  return flag;
}
