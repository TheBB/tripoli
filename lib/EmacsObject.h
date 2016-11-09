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
PyObject *emacs_object(emacs_value val);

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

DOCSTRING(EmacsObject_is_int,
          "obj.is_int()\n\n"
          "Check whether the object is an integer.\n\n"
          "Equivalent to (integerp obj) in elisp.");
PyObject *EmacsObject_is_int(PyObject *self);

DOCSTRING(EmacsObject_is_float,
          "obj.is_float()\n\n"
          "Check whether the object is a float.\n\n"
          "Equivalent to (floatp obj) in elisp.");
PyObject *EmacsObject_is_float(PyObject *self);

DOCSTRING(EmacsObject_is_str,
          "obj.is_str()\n\n"
          "Check whether the object is a string.\n\n"
          "Equivalent to (stringp obj) in elisp.");
PyObject *EmacsObject_is_str(PyObject *self);

DOCSTRING(EmacsObject_is_symbol,
          "obj.is_symbol()\n\n"
          "Check whether the object is a symbol.\n\n"
          "Equivalent to (symbolp obj) in elisp.");
PyObject *EmacsObject_is_symbol(PyObject *self);

DOCSTRING(EmacsObject_is_cons,
          "obj.is_cons()\n\n"
          "Check whether the object is a cons cell.\n\n"
          "Equivalent to (consp obj) in elisp.");
PyObject *EmacsObject_is_cons(PyObject *self);

DOCSTRING(EmacsObject_is_vector,
          "obj.is_vector()\n\n"
          "Check whether the object is a vector.\n\n"
          "Equivalent to (vectorp obj) in elisp.");
PyObject *EmacsObject_is_vector(PyObject *self);

DOCSTRING(EmacsObject_is_list,
          "obj.is_list()\n\n"
          "Check whether the object is a list.\n\n"
          "Equivalent to (listp obj) in elisp.");
PyObject *EmacsObject_is_list(PyObject *self);

DOCSTRING(EmacsObject_is_callable,
          "obj.is_callable()\n\n"
          "Check whether the object is callable.\n\n"
          "Equivalent to (functionp obj) in elisp.");
PyObject *EmacsObject_is_callable(PyObject *self);

PyTypeObject EmacsObjectType;

#endif /* EMACSOBJECT_H */
