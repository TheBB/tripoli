#include <assert.h>
#include <stdio.h>
#include <wordexp.h>
#include <emacs-module.h>
#include <Python.h>

#include "emacs-interface.h"
#include "error.h"
#include "module.h"
#include "util.h"

#include "main.h"


int emacs_module_init(struct emacs_runtime *ert)
{
    // Check if Emacs version is new enough
    assert(ert->size > 0);
    if ((size_t) ert->size < sizeof(*ert))
        return 1;
    emacs_env *env = ert->get_environment(ert);
    assert(env->size > 0);
    if ((size_t) env->size < sizeof(*env))
        return 2;

    push_env(env);
    populate();

    Py_SetProgramName((wchar_t *)"Tripoli");
    PyImport_AppendInittab("emacs_raw", PyInit_emacs_raw);
    Py_Initialize();

    em_defun(exec_buffer, "tripoli-exec-buffer", 0, 0, true, NULL, __doc_exec_buffer, NULL);
    em_defun(exec_file, "tripoli-exec-file", 1, 1, true, em_str("fPython file: "), __doc_exec_file, NULL);
    em_defun(exec_str, "tripoli-exec-str", 1, 1, false, NULL, __doc_exec_str, NULL);
    em_defun(import_module, "tripoli-import", 1, 1, true, em_str("sModule: "), __doc_import_module, NULL);
    em_defun(exec_tests, "tripoli-test", 0, emacs_variadic_function, true, NULL, __doc_exec_tests, NULL);
    em_defun(exec_repl, "tripoli-repl", 0, 0, true, NULL, __doc_exec_repl, NULL);

    em_provide("libtripoli");

    maybe_run_init();

    pop_env();
    return 0;
}


void maybe_run_init()
{
    if (em_bound_and_true_p(em_intern("tripoli-inhibit-init")))
        return;

    FILE *fp = NULL;
    char *candidates[2] = {"~/.emacs.py", "~/.emacs.d/init.py"};
    char *filename = NULL;

    size_t i;
    wordexp_t w;
    for (i = 0; i < 2 && !fp; i++) {
        filename = candidates[i];
        wordexp(candidates[i], &w, 0);
        for (size_t j = 0; j < w.we_wordc && !fp; j++)
            fp = fopen(w.we_wordv[j], "r");
    }
    if (!fp)
        return;

    int rv = PyRun_SimpleFileEx(fp, filename, 1);
    if (rv < 0)
        em_error("An exception was raised");
}


emacs_value exec_buffer(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    UNUSED(nargs); UNUSED(args); UNUSED(data);
    push_env(env);

    char *code = em_extract_str(em_funcall_0(em_intern("buffer-string")));
    int rv = PyRun_SimpleString(code);
    free(code);
    if (rv < 0) {
        em_error("An exception was raised");
        POP_ENV_AND_RETURN(em__nil);
    }

    POP_ENV_AND_RETURN(em__nil);
}


emacs_value exec_file(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    UNUSED(data);
    push_env(env);

    if (nargs != 1) {
        em_error("Expected one argument");
        POP_ENV_AND_RETURN(em__nil);
    }

    emacs_value em_fn = args[0];
    if (!em_stringp(em_fn)) {
        em_error("Expected a string");
        POP_ENV_AND_RETURN(em__nil);
    }

    char *fn = em_extract_str(em_fn);
    FILE *fp = fopen(fn, "r");

    if (!fp) {
        free(fn);
        em_error("Unable to open file");
        POP_ENV_AND_RETURN(em__nil);
    }

    int rv = PyRun_SimpleFileEx(fp, fn, 1);
    free(fn);
    if (rv < 0) {
        em_error("An exception was raised");
        POP_ENV_AND_RETURN(em__nil);
    }

    POP_ENV_AND_RETURN(em__nil);
}


emacs_value exec_str(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    UNUSED(data);
    push_env(env);

    if (nargs != 1) {
        em_error("Expected one argument");
        POP_ENV_AND_RETURN(em__nil);
    }

    emacs_value em_code = args[0];
    if (!em_stringp(em_code)) {
        em_error("Expected a string");
        POP_ENV_AND_RETURN(em__nil);
    }

    char *code = em_extract_str(em_code);
    int rv = PyRun_SimpleString(code);
    free(code);
    if (rv < 0) {
        em_error("An exception was raised");
        POP_ENV_AND_RETURN(em__nil);
    }

    POP_ENV_AND_RETURN(em__nil);
}


emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    UNUSED(data);
    push_env(env);

    if (nargs != 1) {
        em_error("Expected one argument");
        POP_ENV_AND_RETURN(em__nil);
    }

    emacs_value em_name = args[0];
    if (!em_stringp(em_name)) {
        em_error("Expected a string");
        POP_ENV_AND_RETURN(em__nil);
    }

    char *name = em_extract_str(em_name);
    PyObject *module = PyImport_ImportModule(name);
    free(name);
    if (!module) {
        em_error("Failed to import module");
        POP_ENV_AND_RETURN(em__nil);
    }

    if (propagate_python_error())
        POP_ENV_AND_RETURN(em__nil);

    POP_ENV_AND_RETURN(em__nil);
}


emacs_value exec_tests(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    UNUSED(data);
    push_env(env);

    PyObject *module = PyImport_ImportModule("tripoli.test");
    if (!module) {
        em_error("Failed to import test suite");
        POP_ENV_AND_RETURN(em__nil);
    }

    PyObject *func = PyObject_GetAttrString(module, "run_tests");
    if (!func) {
        em_error("Failed to import test suite");
        POP_ENV_AND_RETURN(em__nil);
    }

    for (ptrdiff_t i = 0; i < nargs; i++) {
        if (!em_stringp(args[i])) {
            em_error("Arguments must be strings");
            POP_ENV_AND_RETURN(em__nil);
        }
    }

    PyObject *arglist = PyTuple_New(nargs);
    for (ptrdiff_t i = 0; i < nargs; i++) {
        char *str = em_extract_str(args[i]);
        PyTuple_SetItem(arglist, i, PyUnicode_FromString(str));
        free(str);
    }

    PyObject *ret = PyObject_CallObject(func, arglist);
    Py_DECREF(arglist);

    if (!PyLong_Check(ret)) {
        em_error("Test suite returned unknown exit code");
        POP_ENV_AND_RETURN(em__nil);
    }
    intmax_t val = PyLong_AsLongLong(ret);
    POP_ENV_AND_RETURN(em_int(val));
}


emacs_value exec_repl(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    UNUSED(nargs); UNUSED(args); UNUSED(data);
    push_env(env);

    PyObject *module = PyImport_ImportModule("tripoli.repl");
    if (!module) {
        em_error("Failed to import repl module");
        POP_ENV_AND_RETURN(em__nil);
    }

    PyObject *func = PyObject_GetAttrString(module, "run_repl");
    if (!func) {
        em_error("Failed to import repl module");
        POP_ENV_AND_RETURN(em__nil);
    }

    PyObject *arglist = PyTuple_New(nargs);
    PyObject_CallObject(func, arglist);
    Py_DECREF(arglist);

    POP_ENV_AND_RETURN(em__nil);
}
