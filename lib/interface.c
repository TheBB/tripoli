#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"


static emacs_env *env;

void set_environment(emacs_env *__env)
{
    env = __env;
}

void em_provide(char *feature_name)
{
    emacs_value provide_fcn = em_intern("provide");
    emacs_value args[] = {env->intern(env, feature_name)};
    env->funcall(env, provide_fcn, 1, args);
}

emacs_value em_intern(char *name)
{
    return env->intern(env, name);
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

    emacs_value ret = env->funcall(env, format_fcn, 2, args);
    ptrdiff_t size;
    env->copy_string_contents(env, ret, NULL, &size);
    char *buffer = (char *)malloc(size * sizeof(char));
    env->copy_string_contents(env, ret, buffer, &size);
    return buffer;
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

char *em_type_as_str(emacs_value val)
{
    emacs_value type_fcn = em_intern("type-of");
    emacs_value symbol = env->funcall(env, type_fcn, 1, &val);
    emacs_value name_fcn = em_intern("symbol-name");
    emacs_value name = env->funcall(env, name_fcn, 1, &symbol);
    ptrdiff_t size;
    env->copy_string_contents(env, name, NULL, &size);
    char *buffer = (char *)malloc(size * sizeof(char));
    env->copy_string_contents(env, name, buffer, &size);
    return buffer;
}

bool em_type_is(emacs_value val, char *type)
{
    char *actual = em_type_as_str(val);
    int ret = !strcmp(actual, type);
    free(actual);
    return ret;
}

emacs_value em_make_function(em_func func, ptrdiff_t min_nargs, ptrdiff_t max_nargs, void *data)
{
    return env->make_function(env, min_nargs, max_nargs, func, "", data);
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

bool em_null(emacs_value val)
{
    return !env->is_not_nil(env, val);
}

bool em_stringp(emacs_value val)
{
    emacs_value stringp_fcn = em_intern("stringp");
    return !em_null(env->funcall(env, stringp_fcn, 1, &val));
}

bool em_listp(emacs_value val)
{
    emacs_value listp_fcn = em_intern("listp");
    return !em_null(env->funcall(env, listp_fcn, 1, &val));
}
