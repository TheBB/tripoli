#include <emacs-module.h>

#ifndef INTERFACE_H
#define INTERFACE_H

void set_environment(emacs_env *env);

void em_provide(char *feature_name);
emacs_value em_intern(char *name);
emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args);
char *em_print_obj(emacs_value obj);
emacs_value em_str(char *str);

bool em_null(emacs_value val);
bool em_stringp(emacs_value val);

#endif /* INTERFACE_H */
