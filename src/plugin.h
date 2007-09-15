struct plugin
{
  const char *name;
};

struct plugin *allocate_plugin(const char *name);

void delete_plugin(struct plugin *p);
