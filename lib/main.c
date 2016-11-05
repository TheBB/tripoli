#include <emacs-module.h>
#include <Python.h>

#include "module.h"
#include "interface.h"

int plugin_is_GPL_compatible;


int emacs_module_init(struct emacs_runtime *ert)
{
    set_environment(ert->get_environment(ert));

    Py_SetProgramName((wchar_t *)"tripoli");
    PyImport_AppendInittab("emacs", &PyInit_emacs);
    Py_Initialize();

    PyObject *trp_module = PyImport_ImportModule("tripoli");
    if (!trp_module) {
        printf("Unable to import the `tripoli' module.\n");
        return 1;
    }

    em_provide("libtripoli");

    return 0;
}
