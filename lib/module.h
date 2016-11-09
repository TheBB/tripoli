#include <emacs-module.h>
#include <Python.h>

#ifndef MODULE_H
#define MODULE_H

emacs_value call_func(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

PyObject *py_intern(PyObject *self, PyObject *args);
PyObject *py_str(PyObject *self, PyObject *args);
PyObject *py_int(PyObject *self, PyObject *args);
PyObject *py_float(PyObject *self, PyObject *args);
PyObject *py_function(PyObject *self, PyObject *args);

extern PyObject *EmacsSignal;
extern PyObject *EmacsThrow;

PyObject *PyInit_emacs();

#endif /* MODULE_H */
