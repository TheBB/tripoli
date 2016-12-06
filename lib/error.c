#include <emacs-module.h>
#include <Python.h>

#include "EmacsObject.h"
#include "interface.h"
#include "module.h"

bool propagate_python_error()
{
    // Catch all exceptions
    // PyErr_Fetch steals the references and clears the error indicator
    PyObject *etype, *eval, *etb;
    PyErr_Fetch(&etype, &eval, &etb);
    if (etype) {
        PyObject *args = PyObject_GetAttrString(eval, "args");
        PyObject *pys, *pyd;
        char *msg;

        // If the exception is of type EmacsSignal or EmacsThrow, and was raised
        // with two arguments, both of which are Emacs objects, we can signal or
        // throw a corresponding non-local exit in Emacs
        if ((etype == EmacsSignal || etype == EmacsThrow)
            && PyObject_TypeCheck(args, &PyTuple_Type)
            && PyTuple_Size(args) >= 2
            && (pys = PyTuple_GetItem(args, 0))
            && PyObject_TypeCheck(pys, &EmacsObjectType)
            && (pyd = PyTuple_GetItem(args, 1))
            && PyObject_TypeCheck(pyd, &EmacsObjectType))
        {
            emacs_value symbol = ((EmacsObject *)pys)->val;
            emacs_value data = ((EmacsObject *)pyd)->val;
            if (etype == EmacsSignal)
                em_signal(symbol, data);
            else if (etype == EmacsThrow)
                em_throw(symbol, data);
            Py_DECREF(etype);
            Py_DECREF(eval);
            Py_DECREF(etb);
            return true;
        }

        // Otherwise, simply signal an error, with the error message equal to
        // the exception argument, if any
        if (!PyArg_ParseTuple(args, "s", &msg))
            msg = "An unknown error occured";
        em_error(msg);

        Py_DECREF(etype);
        Py_DECREF(eval);
        Py_DECREF(etb);
        return true;
    }

    return false;
}

bool propagate_emacs_error()
{
    emacs_env *env = get_environment();

    enum emacs_funcall_exit exit_signal = env->non_local_exit_check(env);
    if (!exit_signal)
        return false;

    emacs_value exit_symbol, exit_data;
    env->non_local_exit_get(env, &exit_symbol, &exit_data);
    env->non_local_exit_clear(env);

    PyObject *type = (exit_signal == emacs_funcall_exit_signal) ? EmacsSignal : EmacsThrow;
    PyObject *args = Py_BuildValue("(NN)",
                                   EmacsObject__make(&EmacsObjectType, exit_symbol),
                                   EmacsObject__make(&EmacsObjectType, exit_data));
    PyErr_SetObject(type, args);
    return true;
}
