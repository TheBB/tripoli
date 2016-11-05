#include <Python.h>

#include "module.h"
#include "interface.h"


PyObject *py_emacsobject(PyTypeObject *type, emacs_value val)
{
    EmacsObject *self = (EmacsObject *)type->tp_alloc(type, 0);
    if (self) {
        self->val = val;
    }

    return (PyObject *)self;
}

void EmacsObject_dealloc(EmacsObject *self) {}

PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return NULL;
}

PyTypeObject EmacsObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "emacs.EmacsObject",
    sizeof(EmacsObject),
    0,
    (destructor)EmacsObject_dealloc,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    Py_TPFLAGS_DEFAULT,
    "Emacs object",
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    EmacsObject_new
};

PyObject *py_message(PyObject *self, PyObject *args)
{
    char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;
    em_message(msg);
    Py_RETURN_NONE;
}

PyObject *py_intern(PyObject *self, PyObject *args)
{
    char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    emacs_value sym = em_intern(name);
    PyObject *ret = py_emacsobject(&EmacsObjectType, sym);
    return ret;
}

PyMethodDef methods[] = {
    {"message", py_message, METH_VARARGS, ""},
    {"intern", py_intern, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL}
};

PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "emacs", NULL, -1, methods, NULL, NULL, NULL, NULL
};

PyObject *PyInit_emacs()
{
    PyObject *m = PyModule_Create(&module);
    if (!m) return NULL;

    if (PyType_Ready(&EmacsObjectType) < 0) return NULL;

    Py_INCREF(&EmacsObjectType);
    PyModule_AddObject(m, "EmacsObject", (PyObject *)&EmacsObjectType);

    return m;
}
