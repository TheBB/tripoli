#include <emacs-module.h>

emacs_env *__env;

void set_environment(emacs_env *env);
void em_provide(char *feature_name);
void em_message(char *msg);
