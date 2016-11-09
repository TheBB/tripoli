#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"


static emacs_env *env = NULL;

void set_environment(emacs_env *__env)
{
    env = __env;
}

emacs_env *get_environment()
{
    return env;
}

void em_provide(char *feature_name)
{
    emacs_value provide = em_intern("provide");
    emacs_value feature = em_intern(feature_name);
    em_funcall_naive(provide, 1, &feature);
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
    emacs_value name = em_funcall_naive(symbol_name, 1, &val);
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
    emacs_value symbol = em_funcall_naive(type_of, 1, &val);
    return em_symbol_name(symbol);
}

bool em_type_is(emacs_value val, char *type)
{
    char *actual = em_type_as_str(val);
    int ret = !strcmp(actual, type);
    free(actual);
    return ret;
}

bool em_truthy(emacs_value val)
{
    return env->is_not_nil(env, val);
}

#define PREDICATE(name) \
    bool em_ ## name(emacs_value val) \
    { \
        emacs_value pred = em_intern(#name); \
        return em_truthy(em_funcall_naive(pred, 1, &val)); \
    }

PREDICATE(integerp)
PREDICATE(floatp)
PREDICATE(stringp)
PREDICATE(symbolp)
PREDICATE(consp)
PREDICATE(vectorp)
PREDICATE(listp)
PREDICATE(functionp)

emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args,
                       enum emacs_funcall_exit *exit_signal,
                       emacs_value *exit_symbol, emacs_value *exit_data)
{
    emacs_value ret = env->funcall(env, func, nargs, args);
    if (exit_signal) {
        *exit_signal = env->non_local_exit_check(env);
        if (*exit_signal) {
            env->non_local_exit_get(env, exit_symbol, exit_data);
            env->non_local_exit_clear(env);
        }
    }
    return ret;
}

emacs_value em_funcall_naive(emacs_value func, int nargs, emacs_value *args)
{
    return em_funcall(func, nargs, args, NULL, NULL, NULL);
}

void em_signal(emacs_value symbol, emacs_value data)
{
    env->non_local_exit_signal(env, symbol, data);
}

void em_throw(emacs_value symbol, emacs_value data)
{
    env->non_local_exit_throw(env, symbol, data);
}

void em_error(char *message)
{
    emacs_value list = em_intern("list");
    emacs_value emsg = em_str(message);
    emacs_value data = em_funcall_naive(list, 1, &emsg);
    em_signal(em_intern("error"), data);
}

char *em_print_obj(emacs_value obj)
{
    emacs_value format_fcn = em_intern("format");
    emacs_value fmt_str = env->make_string(env, "%S", 2);
    emacs_value args[] = {fmt_str, obj};
    emacs_value ret = em_funcall_naive(format_fcn, 2, args);
    return em_extract_str(ret);
}
