#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

static const char SYSRQ_PATH[] = "/proc/sys/kernel/sysrq";
static const char DISABLE_VALUE[] = "0\n";
static const int DISABLE_VALUE_LENGTH = ARRAY_SIZE(DISABLE_VALUE);

int main(const int argc, const char **argv) {
  char old_value[256];
  size_t old_value_length = ARRAY_SIZE(old_value);
  FILE *fp;
  char *ret;

  fp = fopen(SYSRQ_PATH, "r+");

  if (!fp) {
    perror("couldn't open file");
    return 1;
  }

  ret = fgets(old_value, old_value_length, fp);

  if (!ret) {
    perror("couldn't read current value");
    return 2;
  }

  old_value_length = strlen(old_value);

  fprintf(stderr, "before: ");
  system("/sbin/sysctl kernel.sysrq");

  rewind(fp); 

  fputs(DISABLE_VALUE, fp);
  fflush(fp);

  fprintf(stderr, "between: ");
  system("/sbin/sysctl kernel.sysrq");

  rewind(fp); 
  fputs(old_value, fp);
  fflush(fp);

  fprintf(stderr, "after: ");
  system("/sbin/sysctl kernel.sysrq");

  return 0;
}
