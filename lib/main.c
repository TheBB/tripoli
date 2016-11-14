#include <stdio.h>
#include <emacs-module.h>
#include <Python.h>

#include "EmacsObject.h"
#include "module.h"
#include "interface.h"
#include "main.h"

static emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    emacs_value em_name = args[0];
    if (!em_stringp(em_name)) {
        em_error("Expected a string");
        UNSET_ENV_AND_RETURN(NULL);
    }

    char *name = em_extract_str(em_name);
    PyObject *module = PyImport_ImportModule(name);
    free(name);

    if (!module) {
        em_error("Failed to import module");
        UNSET_ENV_AND_RETURN(NULL);
    }

    // Check the error indicator
    if (propagate_python_error()) {
        UNSET_ENV_AND_RETURN(NULL);
    }

    UNSET_ENV_AND_RETURN(em_intern("nil"));
}

static emacs_value run_string(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    emacs_value em_code = args[0];
    if (!em_stringp(em_code)) {
        em_error("Expected a string");
        UNSET_ENV_AND_RETURN(NULL);
    }

    char *code = em_extract_str(em_code);
    int rv = PyRun_SimpleString(code);
    free(code);
    if (rv < 0) {
        em_error("An exception was raised while executing the code");
        UNSET_ENV_AND_RETURN(NULL);
    }

    UNSET_ENV_AND_RETURN(em_intern("nil"));
}

static emacs_value run_file(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    emacs_value em_fn = args[0];
    if (!em_stringp(em_fn)) {
        em_error("Expected a string");
        UNSET_ENV_AND_RETURN(NULL);
    }

    char *fn = em_extract_str(em_fn);
    FILE *fp = fopen(fn, "r");

    if (!fp) {
        free(fn);
        em_error("Unable to open file");
        UNSET_ENV_AND_RETURN(NULL);
    }

    int rv = PyRun_SimpleFileEx(fp, fn, 1);
    free(fn);

    if (rv < 0) {
        em_error("An exception was raised while executing the code");
        UNSET_ENV_AND_RETURN(NULL);
    }

    UNSET_ENV_AND_RETURN(em_intern("nil"));
}

static emacs_value run_buffer(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    emacs_value fcn_string = em_intern("buffer-string");
    emacs_value em_code = em_funcall_naive(fcn_string, 0, NULL);
    char *code = em_extract_str(em_code);

    int rv = PyRun_SimpleString(code);
    free(code);
    if (rv < 0) {
        em_error("An exception was raised while executing the code");
        UNSET_ENV_AND_RETURN(NULL);
    }

    UNSET_ENV_AND_RETURN(em_intern("nil"));
}

int emacs_module_init(struct emacs_runtime *ert)
{
    SET_ENV(ert->get_environment(ert));

    Py_SetProgramName((wchar_t *)"Tripoli");
    PyImport_AppendInittab("emacs_raw", &PyInit_emacs);
    Py_Initialize();

    emacs_value fset = em_intern("fset");

    emacs_value trp_import = em_intern("tripoli-import");
    emacs_value fcn_import = em_function(import_module, 1, 1, __doc_import_module, NULL);
    emacs_value import_args[] = {trp_import, fcn_import};
    em_funcall_naive(fset, 2, import_args);

    emacs_value trp_string = em_intern("tripoli-run-string");
    emacs_value fcn_string = em_function(run_string, 1, 1, __doc_run_string, NULL);
    emacs_value string_args[] = {trp_string, fcn_string};
    em_funcall_naive(fset, 2, string_args);

    emacs_value trp_file = em_intern("tripoli--run-file");
    emacs_value fcn_file = em_function(run_file, 1, 1, __doc_run_file, NULL);
    emacs_value file_args[] = {trp_file, fcn_file};
    em_funcall_naive(fset, 2, file_args);

    emacs_value trp_buffer = em_intern("tripoli--run-buffer");
    emacs_value fcn_buffer = em_function(run_buffer, 0, 0, __doc_run_buffer, NULL);
    emacs_value buffer_args[] = {trp_buffer, fcn_buffer};
    em_funcall_naive(fset, 2, buffer_args);

    // This can't be the correct way to make a function interactive...
    em_eval("(defun tripoli-run-file (fn) (interactive \"f\") (tripoli--run-file fn))");
    em_eval("(defun tripoli-run-buffer () (interactive) (tripoli--run-buffer))");

    em_provide("libtripoli");

    UNSET_ENV;
    return 0;
}
