#include "console_switch.h"

bool console_switch_locked = false;

void lock_console_switch(void)
{
  console_switch_locked = true;
}

void unlock_console_switch(void)
{
}
