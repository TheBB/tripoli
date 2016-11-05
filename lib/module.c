#include <Python.h>

#include "module.h"
#include "interface.h"


static PyObject *py_message(PyObject *self, PyObject *args)
{
    char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;
    em_message(msg);
    Py_RETURN_NONE;
}

static PyMethodDef methods[] = {
    {"message", py_message, METH_VARARGS, "Docstring"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "emacs", NULL, -1, methods, NULL, NULL, NULL, NULL
};

PyObject *PyInit_emacs()
{
    return PyModule_Create(&module);
}
