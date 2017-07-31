#include <emacs-module.h>
#include <Python.h>

#include "util.h"

#ifndef OBJECT_H
#define OBJECT_H


/**
 * \brief A Python object that wraps an Emacs object.
 */
typedef struct {
    PyObject_HEAD
    emacs_value val;
} EmacsObject;

PyObject *EmacsObject__make(PyTypeObject *type, emacs_value val);

/**
 * \brief Coerce a Python object to an Emacs object.
 */
bool EmacsObject__coerce(PyObject *arg, bool prefer_symbol, emacs_value *ret);

PyTypeObject EmacsObjectType;

#endif /* OBJECT_H */
