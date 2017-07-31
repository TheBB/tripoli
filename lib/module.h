#include <Python.h>

#ifndef MODULE_H
#define MODULE_H


/**
 * \brief Entry point for calling Python functions from Emacs.
 *
 * This function can be used as a callback when creating an Emacs function
 * object with em_function. It will wrap all the arguments as Python objects, and
 * then call the Python callable pointed to by the data pointer, catching all
 * errors and converting them to Emacs non-local exit signals as appropriate.
 * The return value must be an Emacs object or None (which is converted to nil).
 */
emacs_value call_function(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

PyObject *PyInit_emacs_raw();
PyTypeObject EmacsObjectType;
extern PyObject *EmacsThrow, *EmacsSignal;


#endif /* MODULE_H */
