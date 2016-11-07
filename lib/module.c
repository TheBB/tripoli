#include <Python.h>

#include "module.h"
#include "interface.h"
#include "EmacsObject.h"

PyObject *py_intern(PyObject *self, PyObject *args)
{
    char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    emacs_value sym = em_intern(name);
    return py_emacsobject(sym);
}

PyObject *py_str(PyObject *self, PyObject *args)
{
    char *str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;
    emacs_value ret = em_str(str);
    return py_emacsobject(ret);
}

PyObject *py_int(PyObject *self, PyObject *args)
{
    long long val;
    if (!PyArg_ParseTuple(args, "L", &val))
        return NULL;
    emacs_value ret = em_int(val);
    return py_emacsobject(ret);
}

PyObject *py_float(PyObject *self, PyObject *args)
{
    double val;
    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;
    emacs_value ret = em_float(val);
    return py_emacsobject(ret);
}

emacs_value call_func(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    set_environment(env);
    PyObject *function = (PyObject *)data;
    PyObject *arglist = PyTuple_New(nargs);
    for (size_t i = 0; i < nargs; i++) {
        PyObject *arg = py_emacsobject(args[i]);
        PyTuple_SetItem(arglist, i, arg);
    }
    PyObject *py_ret = PyObject_CallObject(function, arglist);
    if (py_ret == Py_None) {
        return em_intern("nil");
    }
    else if (!PyObject_TypeCheck(py_ret, &EmacsObjectType)) {
        PyErr_SetString(PyExc_TypeError, "Return value must be an Emacs object or None");
        return NULL;
    }
    emacs_value ret = ((EmacsObject *)py_ret)->val;
    return ret;
}

PyObject *py_function(PyObject *self, PyObject *args)
{
    PyObject *fcn;
    int min_nargs, max_nargs;
    if (!PyArg_ParseTuple(args, "Oii", &fcn, &min_nargs, &max_nargs))
        return NULL;
    if (!PyCallable_Check(fcn)) {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
        return NULL;
    }
    Py_XINCREF(fcn);
    emacs_value func = em_function(call_func, min_nargs, max_nargs, fcn);
    return py_emacsobject(func);
}

PyMethodDef methods[] = {
    {"intern", py_intern, METH_VARARGS, ""},
    {"str", py_str, METH_VARARGS, ""},
    {"int", py_int, METH_VARARGS, ""},
    {"float", py_float, METH_VARARGS, ""},
    {"function", py_function, METH_VARARGS, ""},
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
