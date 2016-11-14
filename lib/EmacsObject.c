#include <Python.h>

#include "interface.h"
#include "module.h"
#include "EmacsObject.h"


#define RETURN_TRUE_IF(pred)                                            \
    do {if (pred) { Py_RETURN_TRUE; } else { Py_RETURN_FALSE; }} while (0)

#define EMACSOBJECT_IS(pytype, predicate)               \
    PyObject *EmacsObject_is_ ## pytype(PyObject *self) \
    {                                                   \
        emacs_value obj = ((EmacsObject *)self)->val;   \
        if (em_ ## predicate(obj))                      \
            Py_RETURN_TRUE;                             \
        Py_RETURN_FALSE;                                \
    }

#define METHOD(name, args)                                              \
    {#name, (PyCFunction)EmacsObject_ ## name, METH_ ## args, __doc_EmacsObject_ ## name}


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
    return em_truthy(val);
}

PyObject *EmacsObject_int(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    if (em_integerp(val)) {
        intmax_t integer = em_extract_int(val);
        Py_ssize_t pyint = Py_SAFE_DOWNCAST(integer, intmax_t, Py_ssize_t);
        return PyLong_FromSsize_t(pyint);
    }
    else if (em_floatp(val)) {
        double dbl = em_extract_float(val);
        return PyLong_FromDouble(dbl);
    }
    else if (em_stringp(val)) {
        char *str = em_extract_str(val);
        PyObject *ret = PyLong_FromString(str, NULL, 10);
        free(str);
        return ret;
    }
    PyErr_SetString(PyExc_TypeError, "Incompatible Emacs object type");
    return NULL;
}

PyObject *EmacsObject_float(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    if (em_integerp(val)) {
        intmax_t integer = em_extract_int(val);
        return PyFloat_FromDouble((double)integer);
    }
    else if (em_floatp(val)) {
        double dbl = em_extract_float(val);
        return PyFloat_FromDouble(dbl);
    }
    else if (em_stringp(val)) {
        char *str = em_extract_str(val);
        PyObject *pystr = PyUnicode_FromString(str);
        free(str);
        if (pystr) {
            PyObject *ret = PyFloat_FromString(pystr);
            Py_DECREF(pystr);
            return ret;
        }
    }
    PyErr_SetString(PyExc_TypeError, "Incompatible Emacs object type");
    return NULL;
}

PyObject *EmacsObject_call(PyObject *self, PyObject *args, PyObject *kwds)
{
    if (kwds && PyDict_Size(kwds) > 0) {
        PyErr_SetString(PyExc_ValueError, "Keyword arguments not allowed");
        return NULL;
    }
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

    enum emacs_funcall_exit signal;
    emacs_value ret, symbol, data;
    ret = em_funcall(func, ilen, e_arglist, &signal, &symbol, &data);

    if (signal) {
        PyObject *type = (signal == emacs_funcall_exit_signal) ? EmacsSignal : EmacsThrow;
        PyObject *args = Py_BuildValue("(NN)", emacs_object(symbol), emacs_object(data));
        PyErr_SetObject(type, args);
        return NULL;
    }

    return emacs_object(ret);
}

PyObject *EmacsObject_str(PyObject *self)
{
    emacs_value obj = ((EmacsObject *)self)->val;
    char *str = em_stringp(obj) ? em_extract_str(obj) : em_print_obj(obj);
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

PyObject *EmacsObject_cmp(PyObject *pa, PyObject *pb, int op)
{
    // Emacs doesn't have a not-equals function, so we just negate equality
    if (op == Py_NE) {
        PyObject *ret = EmacsObject_cmp(pa, pb, Py_EQ);
        if (!ret)
            return NULL;
        bool negated = (ret == Py_False);
        Py_DECREF(ret);
        RETURN_TRUE_IF(negated);
    }

    if (!PyObject_TypeCheck(pb, &EmacsObjectType)) {
        PyErr_SetString(PyExc_TypeError, "Arguments must be Emacs objects");
        return NULL;
    }
    emacs_value a = ((EmacsObject *)pa)->val;
    emacs_value b = ((EmacsObject *)pb)->val;

    // Choose which equality predicate to use based on the types involved. This
    // should make equality behave as close as possible to Python equality.
    if (op == Py_EQ) {
        if (em_numberp(a) && em_numberp(b))
            RETURN_TRUE_IF(em_equal_sign(a, b));

        if (em_stringp(a) && em_stringp(b))
            RETURN_TRUE_IF(em_string_equal(a, b));

        RETURN_TRUE_IF(em_equal(a, b));
    }

    // Strings have their own ordering functions
    if (em_stringp(a) && em_stringp(b)) {
        if (op == Py_LT)
            RETURN_TRUE_IF(em_string_lt(a, b));
        else if (op == Py_LE)
            RETURN_TRUE_IF(!em_string_gt(a, b));
        else if (op == Py_GT)
            RETURN_TRUE_IF(em_string_gt(a, b));
        else if (op == Py_GE)
            RETURN_TRUE_IF(!em_string_lt(a, b));
    }

    // Regular ordering uses number-or-marker-p
    if (em_number_or_marker_p(a) && em_number_or_marker_p(b)) {
        if (op == Py_LT)
            RETURN_TRUE_IF(em_lt(a, b));
        else if (op == Py_LE)
            RETURN_TRUE_IF(em_le(a, b));
        else if (op == Py_GT)
            RETURN_TRUE_IF(em_gt(a, b));
        else if (op == Py_GE)
            RETURN_TRUE_IF(em_ge(a, b));
    }

    PyErr_SetString(PyExc_TypeError, "Incompatible Emacs objects");
    return NULL;
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

EMACSOBJECT_IS(int, integerp)
EMACSOBJECT_IS(float, floatp)
EMACSOBJECT_IS(number, numberp)
EMACSOBJECT_IS(number_or_marker, number_or_marker_p)
EMACSOBJECT_IS(str, stringp)
EMACSOBJECT_IS(symbol, symbolp)
EMACSOBJECT_IS(cons, consp)
EMACSOBJECT_IS(vector, vectorp)
EMACSOBJECT_IS(list, listp)
EMACSOBJECT_IS(callable, functionp)

PyMethodDef EmacsObject_methods[] = {
    METHOD(type, NOARGS),
    METHOD(is_a, VARARGS),
    METHOD(is_int, NOARGS),
    METHOD(is_float, NOARGS),
    METHOD(is_number, NOARGS),
    METHOD(is_number_or_marker, NOARGS),
    METHOD(is_str, NOARGS),
    METHOD(is_symbol, NOARGS),
    METHOD(is_cons, NOARGS),
    METHOD(is_vector, NOARGS),
    METHOD(is_list, NOARGS),
    METHOD(is_callable, NOARGS),
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
    EmacsObject_cmp,                  // tp_richcompare
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
