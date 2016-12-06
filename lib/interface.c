#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface.h"


#define PREDICATE(name, test)                                   \
    bool em_ ## name(emacs_value val)                           \
    {                                                           \
        return em_truthy(em_funcall_1(test, val));        \
    }

#define SIMPLE_PREDICATE(name) PREDICATE(name, #name)

#define EQUALITY(name, lisp)                                    \
    bool em_ ## name(emacs_value a, emacs_value b)              \
    {                                                           \
        emacs_value eq = em_intern(lisp);                       \
        return em_truthy(em_funcall_2(lisp, a, b));       \
    }


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
    em_funcall_1("provide", em_intern(feature_name));
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
    emacs_value name = em_funcall_1("symbol-name", val);
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
    emacs_value symbol = em_funcall_1("type-of", val);
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

SIMPLE_PREDICATE(integerp)
SIMPLE_PREDICATE(floatp)
SIMPLE_PREDICATE(numberp)
PREDICATE(number_or_marker_p, "number-or-marker-p")
SIMPLE_PREDICATE(stringp)
SIMPLE_PREDICATE(symbolp)
SIMPLE_PREDICATE(consp)
SIMPLE_PREDICATE(vectorp)
SIMPLE_PREDICATE(listp)
SIMPLE_PREDICATE(functionp)

emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args)
{
    return env->funcall(env, func, nargs, args);
}

emacs_value em_funcall_n(char *func, int nargs, emacs_value *args)
{
    return em_funcall(em_intern(func), nargs, args);
}

emacs_value em_funcall_0(char *func)
{
    return em_funcall(em_intern(func), 0, NULL);
}

emacs_value em_funcall_1(char *func, emacs_value arg)
{
    return em_funcall(em_intern(func), 1, &arg);
}

emacs_value em_funcall_2(char *func, emacs_value arg1, emacs_value arg2)
{
    emacs_value args[] = {arg1, arg2};
    return em_funcall(em_intern(func), 2, args);
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
    emacs_value emsg = em_str(message);
    emacs_value data = em_funcall_1("list", emsg);
    em_signal(em_intern("error"), data);
}

EQUALITY(eq, "eq")
EQUALITY(eql, "eql")
EQUALITY(equal, "equal")
EQUALITY(equal_sign, "=")
EQUALITY(string_equal, "string-equal")
EQUALITY(lt, "<")
EQUALITY(le, "<=")
EQUALITY(gt, ">")
EQUALITY(ge, ">=")
EQUALITY(string_lt, "string<")
EQUALITY(string_gt, "string>")

emacs_value em_eval(char *c)
{
    emacs_value sexp = em_funcall_1("read", em_str(c));
    if (!sexp)
        return NULL;
    return em_funcall_1("eval", sexp);
}

char *em_print_obj(emacs_value obj)
{
    emacs_value ret = em_funcall_2("format", em_str("%S"), obj);
    return em_extract_str(ret);
}

bool em_interactive(emacs_value func, emacs_value spec)
{
    if (!em_consp(func))
        return false;
    if (!em_eq(em_intern("lambda"), em_funcall_1("car", func)))
        return false;

    emacs_value ispec;
    if (spec)
        ispec = em_funcall_2("list", em_intern("interactive"), spec);
    else
        ispec = em_funcall_1("list", em_intern("interactive"));

    // Skip over lambda and argument list
    func = em_funcall_1("cdr", func);
    emacs_value pfunc = func;
    func = em_funcall_1("cdr", func);

    // Skip over docstring if it's there
    if (em_stringp(em_funcall_1("car", func))) {
        pfunc = func;
        func = em_funcall_1("cdr", func);
    }

    // If looking at an interactive spec, replace it
    if (em_consp(em_funcall_1("car", func)) &&
        em_eq(em_intern("interactive"), em_funcall_1("caar", func)))
    {
        em_funcall_2("setcar", func, ispec);
        return true;
    }

    // If not, add an interactive spec
    emacs_value new_cons = em_funcall_2("cons", ispec, func);
    em_funcall_2("setcdr", pfunc, new_cons);
    return true;
}
