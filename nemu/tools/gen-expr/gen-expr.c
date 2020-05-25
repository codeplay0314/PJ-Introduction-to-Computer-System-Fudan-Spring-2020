#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536]="";

// TODO: implement these functions: choose, gen_rand_op, gen_num, gen_rand_expr
static inline uint32_t choose(uint32_t n) {
  return rand() % n;
  return 0;
}

int module;
static inline int gen_rand_op(int cur){
  switch(module? choose(8): choose(4)) {
  case 0: buf[cur++] = '+'; return cur;
  case 1: buf[cur++] = '-'; return cur;
  case 2: buf[cur++] = '*'; return cur;
  case 3: buf[cur++] = '/'; return cur;
  case 4: buf[cur++] = '&'; buf[cur++] = '&'; return cur;
  case 5: buf[cur++] = '|'; buf[cur++] = '|'; return cur;
  case 6: buf[cur++] = '='; buf[cur++] = '='; return cur;
  case 7: buf[cur++] = '!'; buf[cur++] = '='; return cur;
  }
}

static inline int gen_num(){
  return rand() % 1000;
}

static inline int gen_rand_expr(int cur, int t) {
  if (t <= 0) return cur;
  if (t <= 2)  {
    sprintf(buf + cur, "%d%c", gen_num(), 0); // 生成随机数字
    return strlen(buf); // 数字是表达式
  }
  while (1) {
    switch(choose(11)){ // choose(num)表达式随机取一个数字对num取模
    case 0:
      buf[cur++] = ' ';
      gen_rand_expr(cur, t); // 空格/'-' + 表达式是表达式
      break;
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
      ;int lef = choose(t - 2) + 1;
      cur = gen_rand_expr(cur, lef);
      cur = gen_rand_op(cur); // 产生随机操作符
      cur = gen_rand_expr(cur, t - lef - 1); // <expr> "op" <expr>是表达式
      break;
    case 9:
      buf[cur++] = '(';
      cur = gen_rand_expr(cur, t - 2);
      buf[cur++] = ')'; // "(" <expr> ")"也是表达式
      buf[cur++] = '\0';
      break;
    case 10:
      if (t < 4) continue;
      buf[cur++] = '(';
      switch(choose(2)) {
      case 0: buf[cur++] = '-';
      case 1: buf[cur++] = '!';
      }
      cur = gen_rand_expr(cur, t - 3);
      buf[cur++] = ')'; // "(" <expr> ")"也是表达式
      buf[cur++] = '\0';
      break;
    }
    break;
  }
  return strlen(buf);
}
// TODO: if necessary, try to re-implement main function for better generation of expression


static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int limit = 10;
  if (argc > 2) {
    sscanf(argv[2], "%d", &limit);
  }
  int i;
  for (i = 0;  i < loop; i ++) {
    module = i & 1;
    buf[0]='\0';
	  gen_rand_expr(0, limit);
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    if (!~fscanf(fp, "%d", &result)) continue;
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
