#include <Python.h>

#include "error.h"
#include "module.h"
#include "interface.h"
#include "EmacsObject.h"


#define EQUALITY(pred)                                                  \
    PyObject *py_ ## pred(PyObject *self, PyObject *args)               \
    {                                                                   \
        PyObject *pa, *pb;                                              \
        if (!PyArg_ParseTuple(args, "O!O!",                             \
                              &EmacsObjectType, &pa,                    \
                              &EmacsObjectType, &pb))                   \
        {                                                               \
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
    {#name, (PyCFunction) py_ ## name, (args), __doc_py_ ## name}

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

emacs_value call_func(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    SET_ENV(env);

    PyObject *function = (PyObject *)data;
    PyObject *arglist = PyTuple_New(nargs);
    for (size_t i = 0; i < nargs; i++) {
        PyObject *arg = EmacsObject__make(&EmacsObjectType, args[i]);
        PyTuple_SetItem(arglist, i, arg);
    }

    PyObject *py_ret = PyObject_CallObject(function, arglist);

    // Check the error indicator
    if (propagate_python_error()) {
        UNSET_ENV_AND_RETURN(NULL);
    }

    emacs_value ret;
    if (!EmacsObject__coerce(py_ret, 0, &ret)) {
        em_error("Function failed to return a valid Emacs object");
        UNSET_ENV_AND_RETURN(NULL);
    }

    UNSET_ENV_AND_RETURN(ret);
}

PyObject *py_intern(PyObject *self, PyObject *args)
{
    char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    emacs_value sym = em_intern(name);
    return EmacsObject__make(&EmacsObjectType, sym);
}

PyObject *py_str(PyObject *self, PyObject *args)
{
    char *str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;
    emacs_value ret = em_str(str);
    return EmacsObject__make(&EmacsObjectType, ret);
}

PyObject *py_int(PyObject *self, PyObject *args)
{
    long long val;
    if (!PyArg_ParseTuple(args, "L", &val))
        return NULL;
    emacs_value ret = em_int(val);
    return EmacsObject__make(&EmacsObjectType, ret);
}

PyObject *py_float(PyObject *self, PyObject *args)
{
    double val;
    if (!PyArg_ParseTuple(args, "d", &val))
        return NULL;
    emacs_value ret = em_float(val);
    return EmacsObject__make(&EmacsObjectType, ret);
}

PyObject *py_function(PyObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *fcn;
    ptrdiff_t min_nargs = 0, max_nargs = PTRDIFF_MAX;
    char *keywords[] = {"self", "min_nargs", "max_nargs", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|ii", keywords, &fcn, &min_nargs, &max_nargs))
        return NULL;
    if (!PyCallable_Check(fcn)) {
        PyErr_SetString(PyExc_TypeError, "Parameter must be callable");
        return NULL;
    }

    PyObject *pydoc = PyObject_GetAttrString(fcn, "__doc__");
    char *doc = NULL;
    if (pydoc && PyUnicode_Check(pydoc)) {
        doc = PyUnicode_AsUTF8AndSize(pydoc, NULL);
        if (!doc)
            PyErr_Clear();
    }

    Py_XINCREF(fcn);
    emacs_value func = em_function(call_func, min_nargs, max_nargs, doc, fcn);
    return EmacsObject__make(&EmacsObjectType, func);
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
    METHOD(intern, METH_VARARGS),
    METHOD(str, METH_VARARGS),
    METHOD(int, METH_VARARGS),
    METHOD(float, METH_VARARGS),
    METHOD(function, METH_VARARGS | METH_KEYWORDS),
    METHOD(eq, METH_VARARGS),
    METHOD(eql, METH_VARARGS),
    METHOD(equal, METH_VARARGS),
    METHOD(equal_sign, METH_VARARGS),
    METHOD(string_equal, METH_VARARGS),
    METHOD(lt, METH_VARARGS),
    METHOD(le, METH_VARARGS),
    METHOD(gt, METH_VARARGS),
    METHOD(ge, METH_VARARGS),
    METHOD(string_lt, METH_VARARGS),
    METHOD(string_gt, METH_VARARGS),
    METHOD(integerp, METH_VARARGS),
    METHOD(floatp, METH_VARARGS),
    METHOD(numberp, METH_VARARGS),
    METHOD(number_or_marker_p, METH_VARARGS),
    METHOD(stringp, METH_VARARGS),
    METHOD(symbolp, METH_VARARGS),
    METHOD(consp, METH_VARARGS),
    METHOD(vectorp, METH_VARARGS),
    METHOD(listp, METH_VARARGS),
    METHOD(functionp, METH_VARARGS),
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
