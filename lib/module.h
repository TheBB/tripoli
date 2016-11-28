#include <emacs-module.h>
#include <Python.h>

#include "util.h"

#ifndef MODULE_H
#define MODULE_H

/**
 * \brief Entry point for calling Python functions from Emacs.
 *
 * This function can be used as a callback when creating an Emacs function
 * object with em_function. It will wrap all the arguments as Emacs objects, and
 * then call the Python callable pointed to by the data pointer, catching all
 * errors and converging them to Emacs non-local exit signals as appropriate.
 * The return value must be an Emacs object or None (which is converted to nil).
 */
emacs_value call_func(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(py_intern,
          "intern(name)\n\n"
          "Creates an Emacs object corresponding to the interned symbol with the given\n"
          "name. Equivalent to (intern name) in elisp.");
PyObject *py_intern(PyObject *self, PyObject *args);

DOCSTRING(py_str,
          "str(s)\n\n"
          "Creates an Emacs object of string type.");
PyObject *py_str(PyObject *self, PyObject *args);

DOCSTRING(py_int,
          "int(i)\n\n"
          "Creates an Emacs object of integer type.");
PyObject *py_int(PyObject *self, PyObject *args);

DOCSTRING(py_float,
          "float(i)\n\n"
          "Creates an Emacs object of floating point type.");
PyObject *py_float(PyObject *self, PyObject *args);

DOCSTRING(py_function,
          "function(callback, nargs_min, nargs_max)\n\n"
          "Creates an Emacs function object with the specificed arity, which, when called,\n"
          "will run the given callback function and return its value to the Emacs caller,\n"
          "provided the return value is an Emacs object or None.\n\n"
          "If exceptions of type Signal or Throw are raised with two arguments, the\n"
          "corresponding effect will be propagated to Emacs. All other exceptions are\n"
          "propagated as error signals.");
PyObject *py_function(PyObject *self, PyObject *args);

DOCSTRING(py_eq,
          "eq(a, b)\n\n"
          "Check object equality using eq. Equivalent to (eq a b) in elisp.");
PyObject *py_eq(PyObject *self, PyObject *args);

DOCSTRING(py_eql,
          "eql(a, b)\n\n"
          "Check object equality using eq. Equivalent to (eql a b) in elisp.");
PyObject *py_eql(PyObject *self, PyObject *args);

DOCSTRING(py_equal,
          "equal(a, b)\n\n"
          "Check object equality using equal. Equivalent to (equal a b) in elisp.");
PyObject *py_equal(PyObject *self, PyObject *args);

DOCSTRING(py_equal_sign,
          "equal_sign(a, b)\n\n"
          "Check object equality using =. Equivalent to (= a b) in elisp.");
PyObject *py_equal_sign(PyObject *self, PyObject *args);

DOCSTRING(py_string_equal,
          "string_equal(a, b)\n\n"
          "Check object equality using string-equal. Equivalent to (string-equal a b) in\n"
          "elisp.");
PyObject *py_string_equal(PyObject *self, PyObject *args);

DOCSTRING(py_lt,
          "lt(a, b)\n\n"
          "Equivalent to (< a b) in elisp.");
PyObject *py_lt(PyObject *self, PyObject *args);

DOCSTRING(py_le,
          "le(a, b)\n\n"
          "Equivalent to (<= a b) in elisp.");
PyObject *py_le(PyObject *self, PyObject *args);

DOCSTRING(py_gt,
          "gt(a, b)\n\n"
          "Equivalent to (> a b) in elisp.");
PyObject *py_gt(PyObject *self, PyObject *args);

DOCSTRING(py_ge,
          "ge(a, b)\n\n"
          "Equivalent to (>= a b) in elisp.");
PyObject *py_ge(PyObject *self, PyObject *args);

DOCSTRING(py_string_lt,
          "string_lt(a, b)\n\n"
          "Equivalent to (string< a b) in elisp.");
PyObject *py_string_lt(PyObject *self, PyObject *args);

DOCSTRING(py_string_gt,
          "string_gt(a, b)\n\n"
          "Equivalent to (string> a b) in elisp.");
PyObject *py_string_gt(PyObject *self, PyObject *args);

DOCSTRING(py_integerp,
          "Check whether the object is an Emacs integer.\n\n"
          "Equivalent to (integerp obj) in elisp.");
PyObject *py_integerp(PyObject *self, PyObject *args);

DOCSTRING(py_floatp,
          "Check whether the object is an Emacs float.\n\n"
          "Equivalent to (floatp obj) in elisp.");
PyObject *py_floatp(PyObject *self, PyObject *args);

DOCSTRING(py_numberp,
          "Check whether the object is an Emacs number (integer or float).\n\n"
          "Equivalent to (numberp obj) in elisp.");
PyObject *py_numberp(PyObject *self, PyObject *args);

DOCSTRING(py_number_or_marker_p,
          "Check whether the object is an Emacs number or a marker.\n\n"
          "Equivalent to (number-or-marker-p obj) in elisp.");
PyObject *py_number_or_marker_p(PyObject *self, PyObject *args);

DOCSTRING(py_stringp,
          "Check whether the object is an Emacs string.\n\n"
          "Equivalent to (stringp obj) in elisp.");
PyObject *py_stringp(PyObject *self, PyObject *args);

DOCSTRING(py_symbolp,
          "Check whether the object is an Emacs symbol.\n\n"
          "Equivalent to (symbolp obj) in elisp.");
PyObject *py_symbolp(PyObject *self, PyObject *args);

DOCSTRING(py_consp,
          "Check whether the object is an Emacs cons cell.\n\n"
          "Equivalent to (consp obj) in elisp.");
PyObject *py_consp(PyObject *self, PyObject *args);

DOCSTRING(py_vectorp,
          "Check whether the object is an Emacs vector.\n\n"
          "Equivalent to (vectorp obj) in elisp.");
PyObject *py_vectorp(PyObject *self, PyObject *args);

DOCSTRING(py_listp,
          "Check whether the object is an Emacs list.\n\n"
          "Equivalent to (listp obj) in elisp.");
PyObject *py_listp(PyObject *self, PyObject *args);

DOCSTRING(py_functionp,
          "Check whether the object is an Emacs function.\n\n"
          "Equivalent to (functionp obj) in elisp.");
PyObject *py_functionp(PyObject *self, PyObject *args);

extern PyObject *EmacsSignal;
extern PyObject *EmacsThrow;

PyObject *PyInit_emacs();

#endif /* MODULE_H */
