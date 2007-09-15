#include <stdbool.h>

extern bool console_switch_locked;

bool lock_console_switch(char **error);
void unlock_console_switch(void);
