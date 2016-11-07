#include <emacs-module.h>
#include <Python.h>

#include "module.h"
#include "interface.h"

int plugin_is_GPL_compatible;


emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    set_environment(env);

    emacs_value em_name = args[0];
    if (!em_stringp(em_name))
        return em_intern("nil");
    ptrdiff_t size;
    env->copy_string_contents(env, em_name, NULL, &size);
    char *name = (char *)malloc(size * sizeof(char));
    env->copy_string_contents(env, em_name, name, &size);

    PyObject *module = PyImport_ImportModule(name);
    free(name);
    if (!module)
        return em_intern("nil");

    return em_intern("t");
}


int emacs_module_init(struct emacs_runtime *ert)
{
    set_environment(ert->get_environment(ert));

    Py_SetProgramName((wchar_t *)"Tripoli");
    PyImport_AppendInittab("emacs", &PyInit_emacs);
    Py_Initialize();

    emacs_env *env = ert->get_environment(ert);

    emacs_value fset_fcn = em_intern("fset");
    emacs_value tripoli_import = em_intern("tripoli-import");
    emacs_value import_fcn = env->make_function(
        env, 1, 1, import_module, "Import a Python module via Tripoli", NULL);
    emacs_value args[] = {tripoli_import, import_fcn};
    env->funcall(env, fset_fcn, 2, args);

    em_provide("libtripoli");

    return 0;
}
