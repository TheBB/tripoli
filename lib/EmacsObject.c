#include <Python.h>

#include "interface.h"
#include "EmacsObject.h"

void EmacsObject_dealloc(EmacsObject *self) {}

PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return py_emacsobject(em_intern("nil"));
}

int EmacsObject_bool(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    return !em_null(val);
}

PyObject *EmacsObject_int(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    if (em_type_is(val, "integer")) {
        intmax_t integer = em_extract_int(val);
        Py_ssize_t pyint = Py_SAFE_DOWNCAST(integer, intmax_t, Py_ssize_t);
        return PyLong_FromSsize_t(pyint);
    }
    else if (em_type_is(val, "float")) {
        double dbl = em_extract_float(val);
        return PyLong_FromDouble(dbl);
    }
    PyErr_SetString(PyExc_TypeError, "Incompatible Emacs object type");
    return NULL;
}

PyObject *EmacsObject_float(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    if (em_type_is(val, "integer")) {
        intmax_t integer = em_extract_int(val);
        return PyFloat_FromDouble((double)integer);
    }
    else if (em_type_is(val, "float")) {
        double dbl = em_extract_float(val);
        return PyLong_FromDouble(dbl);
    }
    PyErr_SetString(PyExc_TypeError, "Incompatible Emacs object type");
    return NULL;
}

PyObject *EmacsObject_call(PyObject *self, PyObject *args, PyObject *kwds)
{
    emacs_value func = ((EmacsObject *)self)->val;
    Py_ssize_t len = PyTuple_Size(args);
    emacs_value e_arglist[len];
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject *arg = PyTuple_GetItem(args, i);
        if (!PyObject_TypeCheck(arg, &EmacsObjectType)) {
            PyErr_SetString(PyExc_TypeError, "Arguments must be Emacs objects");
            return NULL;
        }
        e_arglist[i] = ((EmacsObject *)arg)->val;
    }
    int ilen = Py_SAFE_DOWNCAST(len, Py_ssize_t, int);
    emacs_value ret = em_funcall(func, ilen, e_arglist);
    return py_emacsobject(ret);
}

PyObject *EmacsObject_str(PyObject *self)
{
    emacs_value obj = ((EmacsObject *)self)->val;
    char *str;
    if (em_type_is(obj, "string"))
        str = em_extract_str(obj);
    else
        str = em_print_obj(obj);
    PyObject *ret = PyUnicode_FromString(str);
    free(str);
    return ret;
}

PyObject *EmacsObject_repr(PyObject *self)
{
    emacs_value obj = ((EmacsObject *)self)->val;
    char *str = em_print_obj(obj);
    PyObject *ret = PyUnicode_FromString(str);
    free(str);
    return ret;
}

PyObject *EmacsObject_type(PyObject *self)
{
    emacs_value obj = ((EmacsObject *)self)->val;
    char *str = em_type_as_str(obj);
    PyObject *ret = PyUnicode_FromString(str);
    free(str);
    return ret;
}

PyObject *EmacsObject_is_a(PyObject *self, PyObject *args)
{
    char *type;
    if (!PyArg_ParseTuple(args, "s", &type))
        return NULL;
    emacs_value obj = ((EmacsObject *)self)->val;
    if (em_type_is(obj, type))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

#define EMACSOBJECT_IS(pytype, emtype) \
    PyObject *EmacsObject_is_ ## pytype(PyObject *self) \
    { \
        emacs_value obj = ((EmacsObject *)self)->val; \
        if (em_type_is(obj, #emtype)) \
            Py_RETURN_TRUE; \
        Py_RETURN_FALSE; \
    }

EMACSOBJECT_IS(int, integer)
EMACSOBJECT_IS(float, float)
EMACSOBJECT_IS(str, string)
EMACSOBJECT_IS(symbol, symbol)
EMACSOBJECT_IS(cons, cons)
EMACSOBJECT_IS(vector, vector)

PyObject *EmacsObject_is_list(PyObject *self)
{
    emacs_value obj = ((EmacsObject *)self)->val;
    if (em_listp(obj))
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

PyMethodDef EmacsObject_methods[] = {
    {"type", (PyCFunction)EmacsObject_type, METH_NOARGS, ""},
    {"is_a", (PyCFunction)EmacsObject_is_a, METH_VARARGS, ""},
    {"is_int", (PyCFunction)EmacsObject_is_int, METH_NOARGS, ""},
    {"is_float", (PyCFunction)EmacsObject_is_float, METH_NOARGS, ""},
    {"is_str", (PyCFunction)EmacsObject_is_str, METH_NOARGS, ""},
    {"is_symbol", (PyCFunction)EmacsObject_is_symbol, METH_NOARGS, ""},
    {"is_cons", (PyCFunction)EmacsObject_is_cons, METH_NOARGS, ""},
    {"is_vector", (PyCFunction)EmacsObject_is_vector, METH_NOARGS, ""},
    {"is_list", (PyCFunction)EmacsObject_is_list, METH_NOARGS, ""},
    {NULL},
};

static PyNumberMethods EmacsObject_NumMethods[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    (inquiry)EmacsObject_bool,
    0, 0, 0, 0, 0, 0,
    EmacsObject_int,
    0,
    EmacsObject_float,
};

PyTypeObject EmacsObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "emacs.EmacsObject",
    sizeof(EmacsObject),
    0,
    (destructor)EmacsObject_dealloc,
    0, 0, 0, 0,
    EmacsObject_repr,
    EmacsObject_NumMethods,
    0, 0, 0,
    EmacsObject_call,
    EmacsObject_str,
    0, 0, 0,
    Py_TPFLAGS_DEFAULT,
    "Emacs object",
    0, 0, 0, 0, 0, 0,
    EmacsObject_methods, 0,
    0, 0, 0, 0, 0, 0,
    0, 0,
    EmacsObject_new,
};

PyObject *py_emacsobject(emacs_value val)
{
    PyTypeObject *type = &EmacsObjectType;
    EmacsObject *self = (EmacsObject *)type->tp_alloc(type, 0);
    if (self)
        self->val = val;
    return (PyObject *)self;
}
