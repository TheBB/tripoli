#include <Python.h>

#include "module.h"
#include "interface.h"
#include "EmacsObject.h"


#define EQUALITY(pred)                                                  \
    PyObject *py_ ## pred(PyObject *self, PyObject *args)               \
    {                                                                   \
        PyObject *pa, *pb;                                              \
        if (!PyArg_ParseTuple(args, "OO", &pa, &pb))                    \
            return NULL;                                                \
        if (!PyObject_TypeCheck(pa, &EmacsObjectType) ||                \
            !PyObject_TypeCheck(pb, &EmacsObjectType))                  \
        {                                                               \
            PyErr_SetString(PyExc_TypeError, "Arguments must be Emacs objects"); \
            return NULL;                                                \
        }                                                               \
        emacs_value a, b;                                               \
        a = ((EmacsObject *)pa)->val;                                   \
        b = ((EmacsObject *)pb)->val;                                   \
        if (em_ ## pred(a, b))                                          \
            Py_RETURN_TRUE;                                             \
        Py_RETURN_FALSE;                                                \
    }

#define METHOD(name, args)                                      \
    {#name, py_ ## name, METH_ ## args, __doc_py_ ## name}

#define TYPECHECK(name)                                         \
    PyObject *py_ ## name(PyObject *self, PyObject *args)       \
    {                                                           \
        PyObject *arg;                                          \
        if (!PyArg_ParseTuple(args, "O", &arg))                 \
            return NULL;                                        \
        if (!PyObject_TypeCheck(arg, &EmacsObjectType))         \
            Py_RETURN_FALSE;                                    \
        emacs_value val = ((EmacsObject *)arg)->val;            \
        if (em_ ## name(val))                                   \
            Py_RETURN_TRUE;                                     \
        Py_RETURN_FALSE;                                        \
    }

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

emacs_value call_func(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    PyObject *function = (PyObject *)data;
    PyObject *arglist = PyTuple_New(nargs);
    for (size_t i = 0; i < nargs; i++) {
        PyObject *arg = emacs_object(args[i]);
        PyTuple_SetItem(arglist, i, arg);
    }

    PyObject *py_ret = PyObject_CallObject(function, arglist);

    // Check the error indicator
    if (propagate_python_error()) {
        UNSET_ENV_AND_RETURN(NULL);
    }

    // The return value must be an Emacs object, but we allow ourselves the
    // convenience of mapping None to nil
    if (py_ret == Py_None) {
        UNSET_ENV_AND_RETURN(em_intern("nil"));
    }
    else if (!PyObject_TypeCheck(py_ret, &EmacsObjectType)) {
        em_error("Function failed to return a valid Emacs object");
        UNSET_ENV_AND_RETURN(NULL);
    }

    UNSET_ENV_AND_RETURN(((EmacsObject *)py_ret)->val);
}

PyObject *py_intern(PyObject *self, PyObject *args)
{
    char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    emacs_value sym = em_intern(name);
    return emacs_object(sym);
}

PyObject *py_str(PyObject *self, PyObject *args)
{
    char *str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;
    emacs_value ret = em_str(str);
    return emacs_object(ret);
}

PyObject *py_int(PyObject *self, PyObject *args)
{
    long long val;
    if (!PyArg_ParseTuple(args, "L", &val))
        return NULL;
    emacs_value ret = em_int(val);
    return emacs_object(ret);
}

PyObject *py_float(PyObject *self, PyObject *args)
{
    double val;
    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;
    emacs_value ret = em_float(val);
    return emacs_object(ret);
}

PyObject *py_function(PyObject *self, PyObject *args)
{
    PyObject *fcn;
    int min_nargs, max_nargs;
    if (!PyArg_ParseTuple(args, "Oii", &fcn, &min_nargs, &max_nargs))
        return NULL;
    if (!PyCallable_Check(fcn)) {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
        return NULL;
    }
    Py_XINCREF(fcn);
    emacs_value func = em_function(call_func, min_nargs, max_nargs, NULL, fcn);
    return emacs_object(func);
}

EQUALITY(eq)
EQUALITY(eql)
EQUALITY(equal)
EQUALITY(equal_sign)
EQUALITY(string_equal)
EQUALITY(lt)
EQUALITY(le)
EQUALITY(gt)
EQUALITY(ge)
EQUALITY(string_lt)
EQUALITY(string_gt)

TYPECHECK(integerp)
TYPECHECK(floatp)
TYPECHECK(numberp)
TYPECHECK(number_or_marker_p)
TYPECHECK(stringp)
TYPECHECK(symbolp)
TYPECHECK(consp)
TYPECHECK(vectorp)
TYPECHECK(listp)
TYPECHECK(functionp)

PyMethodDef methods[] = {
    METHOD(intern, VARARGS),
    METHOD(str, VARARGS),
    METHOD(int, VARARGS),
    METHOD(float, VARARGS),
    METHOD(function, VARARGS),
    METHOD(eq, VARARGS),
    METHOD(eql, VARARGS),
    METHOD(equal, VARARGS),
    METHOD(equal_sign, VARARGS),
    METHOD(string_equal, VARARGS),
    METHOD(lt, VARARGS),
    METHOD(le, VARARGS),
    METHOD(gt, VARARGS),
    METHOD(ge, VARARGS),
    METHOD(string_lt, VARARGS),
    METHOD(string_gt, VARARGS),
    METHOD(integerp, VARARGS),
    METHOD(floatp, VARARGS),
    METHOD(numberp, VARARGS),
    METHOD(number_or_marker_p, VARARGS),
    METHOD(stringp, VARARGS),
    METHOD(symbolp, VARARGS),
    METHOD(consp, VARARGS),
    METHOD(vectorp, VARARGS),
    METHOD(listp, VARARGS),
    METHOD(functionp, VARARGS),
    {NULL},
};

PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "emacs_raw", NULL, -1, methods, NULL, NULL, NULL, NULL
};

PyObject *EmacsSignal = NULL;
PyObject *EmacsThrow = NULL;

PyObject *PyInit_emacs()
{
    PyObject *m = PyModule_Create(&module);
    if (!m) return NULL;

    if (PyType_Ready(&EmacsObjectType) < 0) return NULL;

    Py_INCREF(&EmacsObjectType);
    PyModule_AddObject(m, "EmacsObject", (PyObject *)&EmacsObjectType);

    EmacsSignal = PyErr_NewException("emacs_raw.Signal", NULL, NULL);
    Py_INCREF(EmacsSignal);
    PyModule_AddObject(m, "Signal", EmacsSignal);

    EmacsThrow = PyErr_NewException("emacs_raw.Throw", NULL, NULL);
    Py_INCREF(EmacsThrow);
    PyModule_AddObject(m, "Throw", EmacsThrow);

    return m;
}
