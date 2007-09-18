#include <stdbool.h>
#include <sys/types.h>

bool wait_for_death(pid_t pid, long sec, long usec);
void ensure_death(pid_t pid);
void close_all_fds(void);
