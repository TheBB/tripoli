#include <emacs-module.h>

#ifndef INTERFACE_H
#define INTERFACE_H

emacs_env *__env;

void set_environment(emacs_env *env);
void em_provide(char *feature_name);
void em_message(char *msg);

emacs_value em_intern(char *name);

#endif /* INTERFACE_H */
