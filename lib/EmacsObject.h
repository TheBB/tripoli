#include "util.h"
#include "emacs-module.h"

#ifndef EMACSOBJECT_H
#define EMACSOBJECT_H

/**
 * \brief A Python object that wraps an Emacs object.
 */
typedef struct {
    PyObject_HEAD
    emacs_value val;
} EmacsObject;

/**
 * \brief Create a new EmacsObject.
 */
PyObject *emacs_object(PyTypeObject *type, emacs_value val);

void EmacsObject_dealloc(EmacsObject *self);
PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
int EmacsObject_bool(PyObject *self);
PyObject *EmacsObject_int(PyObject *self);
PyObject *EmacsObject_float(PyObject *self);
PyObject *EmacsObject_call(PyObject *self, PyObject *args, PyObject *kwds);
PyObject *EmacsObject_str(PyObject *self);
PyObject *EmacsObject_repr(PyObject *self);

DOCSTRING(EmacsObject_type,
          "obj.type()\n\n"
          "Return a string representation of the Emacs type of this object.\n\n"
          "Equivalent to (symbol-name (type-of obj)) in elisp.");
PyObject *EmacsObject_type(PyObject *self);

DOCSTRING(EmacsObject_is_a,
          "obj.is_a(type)\n\n"
          "Check whether the object has the given Emacs type.\n\n"
          "Equivalent to (eq (type-of obj) (intern type)) in elisp.");
PyObject *EmacsObject_is_a(PyObject *self, PyObject *args);

PyTypeObject EmacsObjectType;

#endif /* EMACSOBJECT_H */
