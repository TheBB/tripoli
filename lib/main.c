#include <emacs-module.h>
#include <Python.h>

#include "module.h"
#include "interface.h"
#include "main.h"

static emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    emacs_value em_name = args[0];
    if (!em_stringp(em_name))
        return em_intern("nil");
    ptrdiff_t size;
    env->copy_string_contents(env, em_name, NULL, &size);
    char *name = (char *)malloc(size * sizeof(char));
    env->copy_string_contents(env, em_name, name, &size);

    PyObject *module = PyImport_ImportModule(name);
    free(name);
    emacs_value ret = module ? em_intern("t") : em_intern("nil");

    UNSET_ENV();
    return ret;
}

int emacs_module_init(struct emacs_runtime *ert)
{
    SET_ENV(ert->get_environment(ert));

    Py_SetProgramName((wchar_t *)"Tripoli");
    PyImport_AppendInittab("emacs", &PyInit_emacs);
    Py_Initialize();

    emacs_value fset = em_intern("fset");
    emacs_value tripoli_import = em_intern("tripoli-import");
    emacs_value import = em_function(import_module, 1, 1, __doc_import_module, NULL);
    emacs_value args[] = {tripoli_import, import};
    em_funcall(fset, 2, args);

    em_provide("libtripoli");

    UNSET_ENV();
    return 0;
}
