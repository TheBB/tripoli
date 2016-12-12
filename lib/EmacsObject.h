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
PyObject *EmacsObject__make(PyTypeObject *type, emacs_value val);

/**
 * \brief Coerce a Python object to an Emacs object.
 */
bool EmacsObject__coerce(PyObject *arg, int prefer_symbol, emacs_value *ret);

void EmacsObject_dealloc(EmacsObject *self);
PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
int EmacsObject_bool(PyObject *self);
PyObject *EmacsObject_int(PyObject *self);
PyObject *EmacsObject_float(PyObject *self);
PyObject *EmacsObject_call(PyObject *self, PyObject *args, PyObject *kwds);
PyObject *EmacsObject_str(PyObject *self);
PyObject *EmacsObject_repr(PyObject *self);

DOCSTRING(EmacsObject_type,
          "type()\n\n"
          "Return a string representation of the Emacs type of this object.\n\n"
          "Equivalent to :lisp:`(symbol-name (type-of obj))` in elisp.");
PyObject *EmacsObject_type(PyObject *self);

DOCSTRING(EmacsObject_is_a,
          "is_a(type)\n\n"
          "Check whether the object has the given Emacs type.\n\n"
          "Equivalent to :lisp:`(eq (type-of obj) (intern type))` in elisp.");
PyObject *EmacsObject_is_a(PyObject *self, PyObject *args);

DOCSTRING(EmacsObject,
          "EmacsObject(obj, prefer_symbol=False, require_symbol=False)\n\n"
          "Creates an Emacs lisp object from `obj`, according to the following rules.\n\n"
          "1. If its argument is an :class:`.EmacsObject` it will wrap the same underlying "
          "Emacs object.\n\n"
          "   .. note::\n"
          "      This creates a *new Python object* that wraps the *same* "
          "Emacs object, however :class:`.EmacsObject` instances are immutable so that "
          "hardly matters.\n\n"
          "2. If the argument has an :code:`__emacs__` magic method, it is called. "
          "This method *must* return an :class:`.EmacsObject` instance, and it must also accept "
          "the `prefer_symbol` keyword argument.\n\n"
          "   .. note::\n"
          "      It is up to the implementation of each class to decide on the "
          "appropriate meaning of `prefer_symbol` in this case.\n\n"
          "3. :code:`None` and :code:`False` become :code:`nil`, "
          "while :code:`True` becomes :code:`t`.\n"
          "4. Integers and floating point numbers become their obvious Emacs lisp counterparts.\n"
          "5. Strings become strings unless `prefer_symbol` is true, in which case they become symbols.\n"
          "6. Two-element tuples become cons cells. Tuples of other sizes will yield errors. "
          "The cons cell entries are coerced recursively.\n"
          "7. Lists become lists, the elements of which are coerced recursively.\n"
          "8. Anything else yields an error.\n\n"
          ".. note::\n"
          "   Since Emacs lisp has so many different structures for mappings, there is no "
          "automatic coercion for dicts.\n\n"
          "If `require_symbol` is true and the coerced object is not a symbol, an error is thrown.");

PyTypeObject EmacsObjectType;

#endif /* EMACSOBJECT_H */
