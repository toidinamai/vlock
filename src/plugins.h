int load_plugin(const char *name, const char *plugin_dir);
void unload_plugins(void);

enum plugin_hook_t {
  HOOK_VLOCK_START = 0,
  HOOK_VLOCK_END,
  HOOK_VLOCK_SAVE,
  HOOK_VLOCK_SAVE_ABORT,
};

#define NR_HOOKS 4

extern char const *hook_names[NR_HOOKS];

int plugin_hook(enum plugin_hook_t hook);
