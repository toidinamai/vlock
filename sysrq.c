#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define OLD_VALUE_LENGTH 256

static const char SYSRQ_PATH[] = "/proc/sys/kernel/sysrq";
static const char SYSRQ_DISABLE_VALUE[] = "0\n";

static FILE *sysrq_file = NULL;

static char old_value[OLD_VALUE_LENGTH];

int disable_sysrq(void) {
  sysrq_file = fopen(SYSRQ_PATH, "r+");

  if (!sysrq_file) {
    fprintf(stderr, "Warning: couldn't open '%s': %s\n", SYSRQ_PATH, strerror(errno));
    return;
  }

  if (!fgets(old_value, OLD_VALUE_LENGTH, sysrq_file)) {
    fprintf(stderr, "Warning: couldn't get current sysrq setting, won't disable sysrq: %s\n", strerror(errno));
    goto error;
  }

  rewind(sysrq_file);

  if (
    !fputs(SYSRQ_DISABLE_VALUE, sysrq_file)
    || !(ftruncate(fileno(sysrq_file), ftello(sysrq_file)) == 0)
    || !(fflush(sysrq_file) == 0)
    ) {
    fprintf(stderr, "Warning: couldn't disable sysrq: %s\n", strerror(errno));
    goto error;
  }

  rewind(sysrq_file);

  return;
error:
  fclose(sysrq_file);
  sysrq_file = NULL;
}

void restore_sysrq(void) {
  if (!sysrq_file)
    return;
  
  if (!fputs(old_value, sysrq_file)) {
    fprintf(stderr, "Warning: couldn't restore old sysrq setting: %s\n", strerror(errno));
  }

  fclose(sysrq_file);
}

void mask_sysrq(void) {
  if (disable_sysrq())
    atexit(restore_sysrq);
}
