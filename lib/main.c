#include <emacs-module.h>
#include <Python.h>

#include "module.h"
#include "interface.h"


// Emacs complains unless this symbol exists
int plugin_is_GPL_compatible;


/**
 * \brief Emacs-callable function that loads a Python module.
 *
 * Accepts a single string argument, the module name to load. Returns t on
 * success, nil on failure.
 */
static emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    set_environment(env);

    emacs_value em_name = args[0];
    if (!em_type_is(em_name, "string"))
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


/**
 * \brief Initialize the Tripoli library.
 *
 * This starts the embedded Python interpreter, enables the emacs module for it,
 * creates the tripoli-import function in the Emacs runtime and provides the
 * libtripoli feature symbol.
 */
int emacs_module_init(struct emacs_runtime *ert)
{
    set_environment(ert->get_environment(ert));

    Py_SetProgramName((wchar_t *)"Tripoli");
    PyImport_AppendInittab("emacs", &PyInit_emacs);
    Py_Initialize();

    emacs_value fset = em_intern("fset");
    emacs_value tripoli_import = em_intern("tripoli-import");
    emacs_value import = em_function(import_module, 1, 1, NULL);
    emacs_value args[] = {tripoli_import, import};
    em_funcall(fset, 2, args);

    em_provide("libtripoli");

    return 0;
}
