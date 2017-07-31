#include <assert.h>
#include <emacs-module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Python.h>

#include "emacs-interface.h"



// Populate internal objects

#define POPULATE(name, lisp) em__ ## name = em_make_global(em_intern(lisp))
#define SIMPLE_POPULATE(name) POPULATE(name, #name)

void populate()
{
    SIMPLE_POPULATE(nil);
    SIMPLE_POPULATE(t);
    SIMPLE_POPULATE(error);
    SIMPLE_POPULATE(format);
    SIMPLE_POPULATE(list);
    SIMPLE_POPULATE(integerp);
    SIMPLE_POPULATE(floatp);
    SIMPLE_POPULATE(numberp);
    SIMPLE_POPULATE(stringp);
    SIMPLE_POPULATE(symbolp);
    SIMPLE_POPULATE(consp);
    SIMPLE_POPULATE(vectorp);
    SIMPLE_POPULATE(listp);
    SIMPLE_POPULATE(functionp);
    SIMPLE_POPULATE(eval);
    SIMPLE_POPULATE(eq);
    SIMPLE_POPULATE(eql);
    SIMPLE_POPULATE(equal);

    POPULATE(number_or_marker_p, "number-or-marker-p");
    POPULATE(symbol_name, "symbol-name");
    POPULATE(type_of, "type-of");
    POPULATE(equal_sign, "=");
    POPULATE(string_equal, "string-equal");
    POPULATE(lt, "<");
    POPULATE(le, "<=");
    POPULATE(gt, ">");
    POPULATE(ge, ">=");
    POPULATE(string_lt, "string<");
    POPULATE(string_gt, "string>");
}

#undef SIMPLE_POPULATE
#undef POPULATE



// Environment stack

static EnvCons *__env_stack = NULL;

void push_env(emacs_env *env)
{
    EnvCons *new_cons = (EnvCons *)malloc(sizeof(EnvCons));
    new_cons->env = env;
    new_cons->next = __env_stack;
    __env_stack = new_cons;
}

emacs_env *get_env()
{
    assert(__env_stack);
    return __env_stack->env;
}

emacs_env *pop_env()
{
    assert(__env_stack);
    emacs_env *ret = __env_stack->env;
    EnvCons *next = __env_stack->next;
    free(__env_stack);
    __env_stack = next;
    return ret;
}



// Global references

emacs_value em_make_global(emacs_value val)
{
    emacs_env *env = get_env();
    return env->make_global_ref(env, val);
}

void em_free_global(emacs_value val)
{
    emacs_env *env = get_env();
    env->free_global_ref(env, val);
}



// Basic Emacs types

emacs_value em_intern(const char *name)
{
    emacs_env *env = get_env();
    return env->intern(env, name);
}

emacs_value em_str(const char *str)
{
    emacs_env *env = get_env();
    return env->make_string(env, str, strlen(str));
}

emacs_value em_int(intmax_t val)
{
    emacs_env *env = get_env();
    return env->make_integer(env, val);
}

emacs_value em_float(double val)
{
    emacs_env *env = get_env();
    return env->make_float(env, val);
}

emacs_value em_function(emacs_subr func, ptrdiff_t min_nargs, ptrdiff_t max_nargs,
                        const char *doc, void *data)
{
    emacs_env *env = get_env();
    return env->make_function(env, min_nargs, max_nargs, func, doc, data);
}

char *em_symbol_name(emacs_value val)
{
    emacs_value name = em_funcall_1(em__symbol_name, val);
    return em_extract_str(name);
}

char *em_extract_str(emacs_value val)
{
    emacs_env *env = get_env();
    ptrdiff_t size;
    env->copy_string_contents(env, val, NULL, &size);
    char *buffer = (char *)malloc(size * sizeof(char));
    env->copy_string_contents(env, val, buffer, &size);
    return buffer;
}

intmax_t em_extract_int(emacs_value val)
{
    emacs_env *env = get_env();
    return env->extract_integer(env, val);
}

double em_extract_float(emacs_value val)
{
    emacs_env *env = get_env();
    return env->extract_float(env, val);
}



// Predicates

bool em_truthy(emacs_value val)
{
    emacs_env *env = get_env();
    return env->is_not_nil(env, val);
}

#define PREDICATE(name)                                         \
    bool em_ ## name(emacs_value val)                           \
    {                                                           \
        return em_truthy(em_funcall_1(em__ ## name, val));      \
    }

PREDICATE(integerp)
PREDICATE(floatp)
PREDICATE(numberp)
PREDICATE(number_or_marker_p)
PREDICATE(stringp)
PREDICATE(symbolp)
PREDICATE(consp)
PREDICATE(vectorp)
PREDICATE(listp)
PREDICATE(functionp)

#undef PREDICATE

#define COMPARE(name)                                           \
    bool em_ ## name(emacs_value a, emacs_value b)              \
    {                                                           \
        return em_truthy(em_funcall_2(em__ ## name, a, b));     \
    }

COMPARE(eq)
COMPARE(eql)
COMPARE(equal)
COMPARE(equal_sign)
COMPARE(string_equal)
COMPARE(lt)
COMPARE(le)
COMPARE(gt)
COMPARE(ge)
COMPARE(string_lt)
COMPARE(string_gt)

#undef COMPARE

bool em_compare(emacs_value a, emacs_value b, int op, bool *error)
{
    // Emacs doesn't have a not-equals function, so let's just negate equality
    if (op == Py_NE)
        return !em_compare(a, b, Py_EQ, error);

    *error = false;

    // Choose which equality predicate to use based on the types involved. This
    // should make equality behave as close as possible to Python equality.
    if (op == Py_EQ) {
        if (em_numberp(a) && em_numberp(b))
            return em_equal_sign(a, b);
        if (em_stringp(a) && em_stringp(b))
            return em_string_equal(a, b);
        return em_equal(a, b);
    }

    // Strings have their own ordering functions
    if (em_stringp(a) && em_stringp(b)) {
        if (op == Py_LT)
            return em_string_lt(a, b);
        else if (op == Py_LE)
            return !em_string_gt(a, b);
        else if (op == Py_GT)
            return em_string_gt(a, b);
        else if (op == Py_GE)
            return !em_string_lt(a, b);
    }

    // Regular ordering uses number-or-marker-p
    if (em_number_or_marker_p(a) && em_number_or_marker_p(b)) {
        if (op == Py_LT)
            return em_lt(a, b);
        else if (op == Py_LE)
            return em_le(a, b);
        else if (op == Py_GT)
            return em_gt(a, b);
        else if (op == Py_GE)
            return em_ge(a, b);
    }

    *error = true;
    return false;
}



// Emacs function calling interface

emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args)
{
    emacs_env *env = get_env();
    return env->funcall(env, func, nargs, args);
}

emacs_value em_funcall_0(emacs_value func)
{
    return em_funcall(func, 0, NULL);
}

emacs_value em_funcall_1(emacs_value func, emacs_value arg)
{
    return em_funcall(func, 1, &arg);
}

emacs_value em_funcall_2(emacs_value func, emacs_value arg1, emacs_value arg2)
{
    emacs_value args[] = {arg1, arg2};
    return em_funcall(func, 2, args);
}

emacs_value em_funcall_3(emacs_value func, emacs_value arg1, emacs_value arg2, emacs_value arg3)
{
    emacs_value args[] = {arg1, arg2, arg3};
    return em_funcall(func, 3, args);
}



// Non-local exits

void em_signal(emacs_value symbol, emacs_value data)
{
    emacs_env *env = get_env();
    env->non_local_exit_signal(env, symbol, data);
}

void em_throw(emacs_value symbol, emacs_value data)
{
    emacs_env *env = get_env();
    env->non_local_exit_throw(env, symbol, data);
}

void em_error(char *message)
{
    emacs_value emsg = em_str(message);
    emacs_value data = em_funcall_1(em__list, emsg);
    em_signal(em__error, data);
}



// Miscellaneous functions

char *em_print_obj(emacs_value val)
{
    emacs_value ret = em_funcall_2(em__format, em_str("%S"), val);
    return em_extract_str(ret);
}

char *em_type_of(emacs_value val)
{
    emacs_value symbol = em_funcall_1(em__type_of, val);
    return em_symbol_name(symbol);
}

bool em_type_is(emacs_value val, const char *type)
{
    char *actual = em_type_of(val);
    int ret = !strcmp(actual, type);
    free(actual);
    return ret;
}

void em_provide(const char *feature_name)
{
    em_funcall_1(em_intern("provide"), em_intern(feature_name));
}

void em_defun(emacs_subr func, const char *name,
              ptrdiff_t min_nargs, ptrdiff_t max_nargs,
              bool interactive, emacs_value interactive_spec,
              const char *doc, void *data)
{
    emacs_value em_func = em_function(func, min_nargs, max_nargs, NULL, data);
    emacs_value em_name = em_intern(name);
    emacs_value em_doc = doc ? em_str(doc) : NULL;

    emacs_value args[6];
    size_t a = 0;
    args[a++] = em_intern("defun");
    args[a++] = em_name;
    args[a++] = em_funcall_2(em__list, em_intern("&rest"), em_intern("args"));
    if (doc)
        args[a++] = em_doc;
    if (interactive && em_truthy(interactive_spec))
        args[a++] = em_funcall_2(em__list, em_intern("interactive"), interactive_spec);
    else if (interactive)
        args[a++] = em_funcall_1(em__list, em_intern("interactive"));
    args[a++] = em_funcall_3(em__list, em_intern("apply"), em_func, em_intern("args"));
    emacs_value defun_form = em_funcall(em__list, a, args);

    em_funcall_2(em__eval, defun_form, em__t);
}
