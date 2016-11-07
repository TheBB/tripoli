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

bool em_null(emacs_value val)
{
    return !env->is_not_nil(env, val);
}

bool em_stringp(emacs_value val)
{
    emacs_value stringp_fcn = em_intern("stringp");
    return !em_null(env->funcall(env, stringp_fcn, 1, &val));
}
