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
    return py_emacsobject(type, em_intern("nil"));
}

int EmacsObject_bool(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    return !em_null(val);
}

PyObject *EmacsObject_call(PyObject *self, PyObject *args, PyObject *kwds)
{
    emacs_value func = ((EmacsObject *)self)->val;
    Py_ssize_t len = PyTuple_Size(args);
    emacs_value e_arglist[len];
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject *arg = PyTuple_GetItem(args, i);
        if (!PyObject_TypeCheck(arg, &EmacsObjectType)) {
            PyErr_SetString(PyExc_TypeError, "Arguments must be Emacs objects");
            return NULL;
        }
        e_arglist[i] = ((EmacsObject *)arg)->val;
    }
    int ilen = Py_SAFE_DOWNCAST(len, Py_ssize_t, int);
    emacs_value ret = em_funcall(func, ilen, e_arglist);
    return py_emacsobject(&EmacsObjectType, ret);
}

static PyNumberMethods EmacsObject_NumMethods[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    (inquiry)EmacsObject_bool,
};

PyTypeObject EmacsObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "emacs.EmacsObject",
    sizeof(EmacsObject),
    0,
    (destructor)EmacsObject_dealloc,
    0, 0, 0, 0, 0,
    EmacsObject_NumMethods,
    0, 0, 0,
    EmacsObject_call,
    0, 0, 0, 0,
    Py_TPFLAGS_DEFAULT,
    "Emacs object",
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    EmacsObject_new,
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
    return py_emacsobject(&EmacsObjectType, sym);
}

PyMethodDef methods[] = {
    {"message", py_message, METH_VARARGS, ""},
    {"intern", py_intern, METH_VARARGS, ""},
    {NULL},
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
