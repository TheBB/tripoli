#include <emacs-module.h>
#include <Python.h>

#ifndef MODULE_H
#define MODULE_H

typedef struct {
    PyObject_HEAD
    emacs_value val;
} EmacsObject;

PyTypeObject EmacsObjectType;

PyObject *py_emacsobject(PyTypeObject *type, emacs_value val);
void EmacsObject_dealloc(EmacsObject *self);
PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
int EmacsObject_bool(PyObject *self);

PyObject *py_message(PyObject *self, PyObject *args);
PyObject *py_intern(PyObject *self, PyObject *args);

PyObject *PyInit_emacs();

#endif /* MODULE_H */
