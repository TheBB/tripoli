#include <emacs-module.h>

#ifndef INTERFACE_H
#define INTERFACE_H

void set_environment(emacs_env *env);
void em_provide(char *feature_name);
void em_message(char *msg);

emacs_value em_intern(char *name);
emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args);

bool em_null(emacs_value val);

#endif /* INTERFACE_H */
