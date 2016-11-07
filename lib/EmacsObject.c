#include <Python.h>

#include "interface.h"
#include "EmacsObject.h"


PyObject *emacs_object(emacs_value val)
{
    PyTypeObject *type = &EmacsObjectType;
    EmacsObject *self = (EmacsObject *)type->tp_alloc(type, 0);
    if (self)
        self->val = val;
    return (PyObject *)self;
}

void EmacsObject_dealloc(EmacsObject *self) {}

PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    return emacs_object(em_intern("nil"));
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
    return emacs_object(ret);
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

#define METHOD(name, args) \
    {#name, (PyCFunction)EmacsObject_ ## name, METH_ ## args, __doc_EmacsObject_ ## name}

PyMethodDef EmacsObject_methods[] = {
    METHOD(type, NOARGS),
    METHOD(is_a, VARARGS),
    METHOD(is_int, NOARGS),
    METHOD(is_float, NOARGS),
    METHOD(is_str, NOARGS),
    METHOD(is_symbol, NOARGS),
    METHOD(is_cons, NOARGS),
    METHOD(is_vector, NOARGS),
    METHOD(is_list, NOARGS),
    {NULL},
};

static PyNumberMethods EmacsObject_NumMethods[] = {
    0,                                // nb_add
    0,                                // nb_subtract
    0,                                // nb_multiply
    0,                                // nb_remainder
    0,                                // nb_divmod
    0,                                // nb_power
    0,                                // nb_negative
    0,                                // nb_positive
    0,                                // nb_absolute
    (inquiry)EmacsObject_bool,        // nb_bool
    0,                                // nb_invert
    0,                                // nb_lshift
    0,                                // nb_rshift
    0,                                // nb_and
    0,                                // nb_xor
    0,                                // nb_or
    EmacsObject_int,                  // nb_int
    0,                                // nb_reserved
    EmacsObject_float,                // nb_float
    0,                                // nb_inplace_add
    0,                                // nb_inplace_subtract
    0,                                // nb_inplace_multiply
    0,                                // nb_inplace_remainder
    0,                                // nb_inplace_power
    0,                                // nb_inplace_lshift
    0,                                // nb_inplace_rshift
    0,                                // nb_inplace_and
    0,                                // nb_inplace_xor
    0,                                // nb_inplace_or
    0,                                // nb_floor_divide
    0,                                // nb_true_divide
    0,                                // nb_inplace_floor_divide
    0,                                // nb_inplace_true_divide
    0,                                // nb_index
    0,                                // nb_matrix_multiple
    0,                                // nb_inplace_matrix_multiply
};

PyTypeObject EmacsObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "emacs.EmacsObject",              // tp_name
    sizeof(EmacsObject),              // tp_basicsize
    0,                                // tp_itemsize
    (destructor)EmacsObject_dealloc,  // tp_dealloc
    0,                                // tp_print
    0,                                // tp_getattr
    0,                                // tp_setattr
    0,                                // tp_as_async
    EmacsObject_repr,                 // tp_repr
    EmacsObject_NumMethods,           // tp_as_number
    0,                                // tp_as_sequence
    0,                                // tp_as_mapping
    0,                                // tp_hash
    EmacsObject_call,                 // tp_call
    EmacsObject_str,                  // tp_str
    0,                                // tp_getattro
    0,                                // tp_setattro
    0,                                // tp_as_buffer
    Py_TPFLAGS_DEFAULT,               // tp_flags
    "Emacs object",                   // tp_doc
    0,                                // tp_traverse
    0,                                // tp_clear
    0,                                // tp_richcompare
    0,                                // tp_weaklistoffset
    0,                                // tp_iter
    0,                                // tp_internext
    EmacsObject_methods,              // tp_methods
    0,                                // tp_members
    0,                                // tp_getset
    0,                                // tp_base
    0,                                // tp_dict
    0,                                // tp_descr_get
    0,                                // tp_descr_set
    0,                                // tp_dictoffset
    0,                                // tp_init
    0,                                // tp_alloc
    EmacsObject_new,                  // tp_new
    0,                                // tp_free
    0,                                // tp_is_gc
    0,                                // tp_bases
    0,                                // tp_mro
    0,                                // tp_cache
    0,                                // tp_subclasses
    0,                                // tp_weaklist
    0,                                // tp_del
    0,                                // tp_version_tag
    0,                                // tp_finalize
};
