#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "logging.h"

static void vlock_log_handler(const gchar *log_domain,
                              GLogLevelFlags __attribute__((unused)) log_level,
                              const gchar *message,
                              gpointer __attribute__((unused)) user_data)
{
  /* XXX: observe the log level (maybe look at some environment variable) */
  if (log_domain != NULL)
    fprintf(stderr, "%s(%s): %s\n", g_get_prgname(), log_domain, message);
  else
    fprintf(stderr, "%s: %s\n", g_get_prgname(), message);
}

static void vlock_error_log_handler(const gchar *log_domain,
                                    GLogLevelFlags log_level,
                                    const gchar *message,
                                    gpointer __attribute__((unused)) user_data)
{
  vlock_log_handler(log_domain, log_level, message, NULL);
  abort();
}

static void vlock_critical_log_handler(const gchar *log_domain,
                                       GLogLevelFlags log_level,
                                       const gchar *message,
                                       gpointer __attribute__(
                                         (unused)) user_data)
{
  vlock_log_handler(log_domain, log_level, message, NULL);
  exit(1);
}

void vlock_initialize_logging(void)
{
  g_log_set_handler(NULL, G_LOG_LEVEL_ERROR, vlock_error_log_handler, NULL);
  g_log_set_handler(NULL,
                    G_LOG_LEVEL_CRITICAL,
                    vlock_critical_log_handler,
                    NULL);
  g_log_set_default_handler(vlock_log_handler, NULL);
}

