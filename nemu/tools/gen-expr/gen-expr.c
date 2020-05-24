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
  return 0;
}

static inline void gen_rand_op(){
  return;
}

static inline void gen_num(){
  return;
}

static inline void gen_rand_expr() {
  buf[0] = 50;
  buf[1] = '\0';
	return;
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
  int i;
  for (i = 0;  i < loop; i ++) {
    buf[0]='\0';
	  gen_rand_expr();
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
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}


