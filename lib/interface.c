#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"


static emacs_env *env;

void em_provide(char *feature_name)
{
    emacs_value provide = em_intern("provide");
    emacs_value feature = em_intern(feature_name);
    em_funcall(provide, 1, &feature);
}

emacs_value em_intern(char *name)
{
    return env->intern(env, name);
}

emacs_value em_str(char *str)
{
    return env->make_string(env, str, strlen(str));
}

emacs_value em_int(intmax_t val)
{
    return env->make_integer(env, val);
}

emacs_value em_float(double val)
{
    return env->make_float(env, val);
}

emacs_value em_function(em_func func, ptrdiff_t min_nargs, ptrdiff_t max_nargs,
                        const char *doc, void *data)
{
    return env->make_function(env, min_nargs, max_nargs, func, doc, data);
}

char *em_symbol_name(emacs_value val)
{
    emacs_value symbol_name = em_intern("symbol-name");
    emacs_value name = em_funcall(symbol_name, 1, &val);
    return em_extract_str(name);
}

char *em_extract_str(emacs_value val)
{
    ptrdiff_t size;
    env->copy_string_contents(env, val, NULL, &size);
    char *buffer = (char *)malloc(size * sizeof(char));
    env->copy_string_contents(env, val, buffer, &size);
    return buffer;
}

intmax_t em_extract_int(emacs_value val)
{
    return env->extract_integer(env, val);
}

double em_extract_float(emacs_value val)
{
    return env->extract_float(env, val);
}

char *em_type_as_str(emacs_value val)
{
    emacs_value type_of = em_intern("type-of");
    emacs_value symbol = em_funcall(type_of, 1, &val);
    return em_symbol_name(symbol);
}

bool em_type_is(emacs_value val, char *type)
{
    char *actual = em_type_as_str(val);
    int ret = !strcmp(actual, type);
    free(actual);
    return ret;
}

bool em_null(emacs_value val)
{
    return !env->is_not_nil(env, val);
}

bool em_listp(emacs_value val)
{
    emacs_value listp = em_intern("listp");
    return !em_null(em_funcall(listp, 1, &val));
}

void set_environment(emacs_env *__env)
{
    env = __env;
}

emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args)
{
    return env->funcall(env, func, nargs, args);
}

char *em_print_obj(emacs_value obj)
{
    emacs_value format_fcn = em_intern("format");
    emacs_value fmt_str = env->make_string(env, "%S", 2);
    emacs_value args[] = {fmt_str, obj};
    emacs_value ret = em_funcall(format_fcn, 2, args);
    return em_extract_str(ret);
}
