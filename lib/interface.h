#include <emacs-module.h>

#ifndef INTERFACE_H
#define INTERFACE_H

typedef emacs_value (*em_func) (emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *);

void set_environment(emacs_env *env);

void em_provide(char *feature_name);
emacs_value em_intern(char *name);
emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args);
char *em_print_obj(emacs_value obj);
emacs_value em_str(char *str);
emacs_value em_int(intmax_t val);
emacs_value em_float(double val);
char *em_type_as_str(emacs_value val);
bool em_type_is(emacs_value val, char *type);
emacs_value em_make_function(em_func func, ptrdiff_t min_nargs, ptrdiff_t max_nargs, void *data);

char *em_extract_str(emacs_value val);
intmax_t em_extract_int(emacs_value val);
double em_extract_float(emacs_value val);

bool em_null(emacs_value val);
bool em_stringp(emacs_value val);
bool em_listp(emacs_value val);

#endif /* INTERFACE_H */
