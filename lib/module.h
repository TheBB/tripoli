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
          "Creates an :class:`.EmacsObject` corresponding to the interned "
          "symbol with the given name. "
          "Equivalent to :lisp:`(intern name)` in elisp.");
PyObject *py_intern(PyObject *self, PyObject *args);

DOCSTRING(py_str,
          "str(s)\n\n"
          "Creates an :class:`.EmacsObject` of string type.");
PyObject *py_str(PyObject *self, PyObject *args);

DOCSTRING(py_int,
          "int(i)\n\n"
          "Creates an :class:`.EmacsObject` of integer type.");
PyObject *py_int(PyObject *self, PyObject *args);

DOCSTRING(py_float,
          "float(f)\n\n"
          "Creates an :class:`.EmacsObject` of floating point type.");
PyObject *py_float(PyObject *self, PyObject *args);

DOCSTRING(py_function,
          "function(callback, nargs_min=0, nargs_max=PTRDIFF_MAX)\n\n"
          "Creates an Emacs function object with the specificed arity which, when called, "
          "will run the given callback function and return its value to the Emacs caller, "
          "provided the return value is coercible to an :class:`.EmacsObject`.\n\n"
          "If exceptions of type :class:`.Signal` or :class:`.Throw` are raised with "
          "appropriate arguments, the corresponding effect will be propagated to Emacs. "
          "All other exceptions are propagated as error signals.");
PyObject *py_function(PyObject *self, PyObject *args, PyObject *kwds);

DOCSTRING(py_eq,
          "eq(a, b)\n\n"
          "Check object equality using :lisp:`eq`. Equivalent to :lisp:`(eq a b)` in elisp.");
PyObject *py_eq(PyObject *self, PyObject *args);

DOCSTRING(py_eql,
          "eql(a, b)\n\n"
          "Check object equality using :lisp:`eql`. Equivalent to :lisp:`(eql a b)` in elisp.");
PyObject *py_eql(PyObject *self, PyObject *args);

DOCSTRING(py_equal,
          "equal(a, b)\n\n"
          "Check object equality using :lisp:`equal`. Equivalent to :lisp:`(equal a b)` in elisp.");
PyObject *py_equal(PyObject *self, PyObject *args);

DOCSTRING(py_equal_sign,
          "equal_sign(a, b)\n\n"
          "Check object equality using :lisp:`=`. Equivalent to :lisp:`(= a b)` in elisp.");
PyObject *py_equal_sign(PyObject *self, PyObject *args);

DOCSTRING(py_string_equal,
          "string_equal(a, b)\n\n"
          "Check object equality using :lisp:`string-equal`. "
          "Equivalent to :lisp:`(string-equal a b)` in elisp.");
PyObject *py_string_equal(PyObject *self, PyObject *args);

DOCSTRING(py_lt,
          "lt(a, b)\n\n"
          "Equivalent to :lisp:`(< a b)` in elisp.");
PyObject *py_lt(PyObject *self, PyObject *args);

DOCSTRING(py_le,
          "le(a, b)\n\n"
          "Equivalent to :lisp:`(<= a b)` in elisp.");
PyObject *py_le(PyObject *self, PyObject *args);

DOCSTRING(py_gt,
          "gt(a, b)\n\n"
          "Equivalent to :lisp:`(> a b)` in elisp.");
PyObject *py_gt(PyObject *self, PyObject *args);

DOCSTRING(py_ge,
          "ge(a, b)\n\n"
          "Equivalent to :lisp:`(>= a b)` in elisp.");
PyObject *py_ge(PyObject *self, PyObject *args);

DOCSTRING(py_string_lt,
          "string_lt(a, b)\n\n"
          "Equivalent to :lisp:`(string< a b)` in elisp.");
PyObject *py_string_lt(PyObject *self, PyObject *args);

DOCSTRING(py_string_gt,
          "string_gt(a, b)\n\n"
          "Equivalent to :lisp:`(string> a b)` in elisp.");
PyObject *py_string_gt(PyObject *self, PyObject *args);

DOCSTRING(py_integerp,
          "Check whether the object is an Emacs integer. "
          "Equivalent to :lisp:`(integerp obj)` in elisp.");
PyObject *py_integerp(PyObject *self, PyObject *args);

DOCSTRING(py_floatp,
          "floatp(obj)\n\n"
          "Check whether the object is an Emacs float. "
          "Equivalent to :lisp:`(floatp obj)` in elisp.");
PyObject *py_floatp(PyObject *self, PyObject *args);

DOCSTRING(py_numberp,
          "numberp(obj)\n\n"
          "Check whether the object is an Emacs number (integer or float). "
          "Equivalent to :lisp:`(numberp obj)` in elisp.");
PyObject *py_numberp(PyObject *self, PyObject *args);

DOCSTRING(py_number_or_marker_p,
          "number_or_marker_p(obj)\n\n"
          "Check whether the object is an Emacs number or a marker. "
          "Equivalent to :lisp:`(number-or-marker-p obj)` in elisp.");
PyObject *py_number_or_marker_p(PyObject *self, PyObject *args);

DOCSTRING(py_stringp,
          "stringp(obj)\n\n"
          "Check whether the object is an Emacs string. "
          "Equivalent to :lisp:`(stringp obj)` in elisp.");
PyObject *py_stringp(PyObject *self, PyObject *args);

DOCSTRING(py_symbolp,
          "symbolp(obj)\n\n"
          "Check whether the object is an Emacs symbol. "
          "Equivalent to :lisp:`(symbolp obj)` in elisp.");
PyObject *py_symbolp(PyObject *self, PyObject *args);

DOCSTRING(py_consp,
          "consp(obj)\n\n"
          "Check whether the object is an Emacs cons cell. "
          "Equivalent to :lisp:`(consp obj)` in elisp.");
PyObject *py_consp(PyObject *self, PyObject *args);

DOCSTRING(py_vectorp,
          "vectorp(obj)\n\n"
          "Check whether the object is an Emacs vector. "
          "Equivalent to :lisp:`(vectorp obj)` in elisp.");
PyObject *py_vectorp(PyObject *self, PyObject *args);

DOCSTRING(py_listp,
          "listp(obj)\n\n"
          "Check whether the object is an Emacs list. "
          "Equivalent to :lisp:`(listp obj)` in elisp.");
PyObject *py_listp(PyObject *self, PyObject *args);

DOCSTRING(py_functionp,
          "functionp(obj)\n\n"
          "Check whether the object is an Emacs function. "
          "Equivalent to :lisp:`(functionp obj)` in elisp.");
PyObject *py_functionp(PyObject *self, PyObject *args);

DOCSTRING(EmacsSignal,
          "Signal(symbol, data)\n\n"
          "Represents an Emacs signal (errors, mostly). "
          "When raised from Emacs, will have two arguments, a symbol denoting the type of signal "
          "(usually `error`, but not always), and a list with `data`, usually a single string "
          "giving an error message.\n\n");
extern PyObject *EmacsSignal;

DOCSTRING(EmacsThrow,
          "Throw(tag, value)\n\n"
          "Represents an Emacs throw (using the :lisp:`throw` built-in). "
          "When raised from Emacs, will have two arguments, a symbol denoting the `tag` "
          "and an arbitrary object denoting the `value`.");
extern PyObject *EmacsThrow;

PyObject *PyInit_emacs();

#endif /* MODULE_H */
