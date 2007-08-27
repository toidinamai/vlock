int load_plugin(const char *name, const char *plugin_dir);
void unload_plugins(void);
int resolve_dependencies(void);

#define HOOK_VLOCK_START 0
#define HOOK_VLOCK_END 1
#define HOOK_VLOCK_SAVE 2
#define HOOK_VLOCK_SAVE_ABORT 3

#define NR_HOOKS 4
#define MAX_HOOK HOOK_VLOCK_SAVE_ABORT

extern const char *hook_names[NR_HOOKS];

int plugin_hook(unsigned int hook);
