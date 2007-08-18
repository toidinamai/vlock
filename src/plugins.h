int load_plugin(const char *name, const char *plugin_dir);
void unload_plugins(void);

int plugin_hook(const char *name);

/* This hook is invoked at the start of vlock.
 * It is provided with a reference to a pointer
 * to NULL that may be changed to point to an
 * arbitrary context object.
 *
 * On success it should return 0 and a negative
 * number on error.  If this function fails, vlock
 * will not continue.
 */
typedef int (*vlock_start_t)(void **);
int vlock_start(void **);

/* This hook is invoked when vlock is about to
 * exit.  Its argument is a reference to the same
 * pointer previously given to vlock_start.  
 *
 * On success it should return 0 and a negative
 * number on error.  The return value is ignored
 * however.
 */
typedef int (*vlock_end_t)(void **);
int vlock_end(void **);

/* This hook is invoked whenever a timeout
 * is reached or Escape is pressed after locking
 * the screen.  Rules for the argument are the same
 * as for vlcok_start, except that the pointer
 * may already point to valid data if set in a previous
 * run.
 *
 * On success this function should return 0 and a
 * negative value on error.  If this function fails
 * it will not be called the next time.
 */
typedef int (*vlock_save_t)(void **);
int vlock_save(void **);

/* This hook is invoked to abort the previous one.
 * The pointer argument is the same as for vlock_save.
 *
 * On success this function should return 0 and a
 * negative value on error.  If this function fails
 * it will not be called the next time.
 */
typedef int (*vlock_save_abort_t)(void **);
int vlock_save_abort(void **);
