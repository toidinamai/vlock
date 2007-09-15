#include <stdbool.h>

extern bool console_switch_locked;

void lock_console_switch(void);
void unlock_console_switch(void);
