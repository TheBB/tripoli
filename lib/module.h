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

/**
 * \brief Check for Python errors having been thrown.
 *
 * At any time, this function may be called to check whether the Python error
 * indicator is set. If it is, the error is converted to an Emacs non-local exit
 * signal as appropriate, and the Python error indicator is cleared.
 *
 * \return True if an error was present, false otherwise.
 */
bool propagate_python_error();

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

extern PyObject *EmacsSignal;
extern PyObject *EmacsThrow;

PyObject *PyInit_emacs();

#endif /* MODULE_H */
