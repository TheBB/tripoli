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
        UNSET_ENV();
        return NULL;
    }

    char *name = em_extract_str(em_name);
    PyObject *module = PyImport_ImportModule(name);
    free(name);

    if (!module) {
        em_error("Failed to import module");
        UNSET_ENV();
        return NULL;
    }

    // Check the error indicator
    if (propagate_python_error()) {
        UNSET_ENV();
        return NULL;
    }

    emacs_value ret = em_intern("nil");
    UNSET_ENV();
    return ret;
}

static emacs_value run_python(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    emacs_value em_code = args[0];
    if (!em_stringp(em_code)) {
        em_error("Expected a string");
        UNSET_ENV();
        return NULL;
    }

    char *code = em_extract_str(em_code);
    if (PyRun_SimpleString(code) < 0) {
        em_error("An exception was raised while executing the code");
        UNSET_ENV();
        return NULL;
    }

    emacs_value ret = em_intern("nil");
    UNSET_ENV();
    return ret;
}

int emacs_module_init(struct emacs_runtime *ert)
{
    SET_ENV(ert->get_environment(ert));

    Py_SetProgramName((wchar_t *)"Tripoli");
    PyImport_AppendInittab("emacs_raw", &PyInit_emacs);
    Py_Initialize();

    emacs_value fset = em_intern("fset");

    emacs_value tripoli_import = em_intern("tripoli-import");
    emacs_value import = em_function(import_module, 1, 1, __doc_import_module, NULL);
    emacs_value import_args[] = {tripoli_import, import};
    em_funcall_naive(fset, 2, import_args);

    emacs_value tripoli_run = em_intern("tripoli-run");
    emacs_value run = em_function(run_python, 1, 1, __doc_run_python, NULL);
    emacs_value run_args[] = {tripoli_run, run};
    em_funcall_naive(fset, 2, run_args);

    em_provide("libtripoli");

    UNSET_ENV();
    return 0;
}
