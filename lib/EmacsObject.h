#include "emacs-module.h"

#ifndef EMACSOBJECT_H
#define EMACSOBJECT_H

typedef struct {
    PyObject_HEAD
    emacs_value val;
} EmacsObject;

PyTypeObject EmacsObjectType;

PyObject *py_emacsobject(emacs_value val);

#endif /* EMACSOBJECT_H */
