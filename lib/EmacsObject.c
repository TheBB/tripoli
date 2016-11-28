#include <Python.h>

#include "interface.h"
#include "module.h"
#include "EmacsObject.h"


#define RETURN_TRUE_IF(pred)                                            \
    do {if (pred) { Py_RETURN_TRUE; } else { Py_RETURN_FALSE; }} while (0)

#define METHOD(name, args)                                              \
    {#name, (PyCFunction)EmacsObject_ ## name, METH_ ## args, __doc_EmacsObject_ ## name}

#define NORMALIZED_UNARY_OPERATION(name, pyname)                        \
    PyObject *EmacsObject_ ## name(PyObject *self)                      \
    {                                                                   \
        PyObject *c_self = self;                                        \
        if (PyObject_TypeCheck(self, &EmacsObjectType))                 \
            c_self = normalize(c_self);                                 \
                                                                        \
        PyObject *ret = NULL;                                           \
                                                                        \
        if (c_self)                                                     \
            ret = pyname(c_self);                                       \
        else                                                            \
            PyErr_SetString(PyExc_TypeError, "Unsupported operand types"); \
                                                                        \
        if (c_self != self) Py_XDECREF(c_self);                         \
        return ret;                                                     \
    }

#define NORMALIZED_BINARY_OPERATION(name, pyname)                       \
    PyObject *EmacsObject_ ## name(PyObject *self, PyObject *other)     \
    {                                                                   \
        PyObject *c_self = self, *c_other = other;                      \
        if (PyObject_TypeCheck(self, &EmacsObjectType))                 \
            c_self = normalize(c_self);                                 \
        if (PyObject_TypeCheck(other, &EmacsObjectType))                \
            c_other = normalize(c_other);                               \
                                                                        \
        PyObject *ret = NULL;                                           \
                                                                        \
        if (c_self && c_other)                                          \
            ret = pyname(c_self, c_other);                              \
        else                                                            \
            PyErr_SetString(PyExc_TypeError, "Unsupported operand types"); \
                                                                        \
        if (c_self != self) Py_XDECREF(c_self);                         \
        if (c_other != other) Py_XDECREF(c_other);                      \
        return ret;                                                     \
    }

PyObject *emacs_object(PyTypeObject *type, emacs_value val)
{
    EmacsObject *self = (EmacsObject *)type->tp_alloc(type, 0);
    if (self)
        self->val = val;
    return (PyObject *)self;
}

void EmacsObject_dealloc(EmacsObject *self) {}

PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    if (PyTuple_Size(args) == 0)
        return emacs_object(type, em_intern("nil"));
    PyObject *arg;
    int prefer_symbol = false;
    char *keywords[] = {"obj", "prefer_symbol", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|$p", keywords, &arg, &prefer_symbol))
        return NULL;
    if (PyObject_TypeCheck(arg, &EmacsObjectType)) {
        emacs_value val = ((EmacsObject *)arg)->val;
        return emacs_object(type, val);
    }
    if (arg == Py_None || arg == Py_False)
        return emacs_object(type, em_intern("nil"));
    if (arg == Py_True)
        return emacs_object(type, em_intern("t"));
    if (PyLong_Check(arg)) {
        int overflow;
        long long val = PyLong_AsLongLongAndOverflow(arg, &overflow);
        if (PyErr_Occurred() || overflow) return NULL;
        return emacs_object(type, em_int(val));
    }
    if (PyFloat_Check(arg)) {
        double val = PyFloat_AsDouble(arg);
        if (PyErr_Occurred()) return NULL;
        return emacs_object(type, em_float(val));
    }
    if (PyUnicode_Check(arg)) {
        char *val = PyUnicode_AsUTF8AndSize(arg, NULL);
        if (!val) return NULL;
        if (prefer_symbol)
            return emacs_object(type, em_intern(val));
        return emacs_object(type, em_str(val));
    }

    PyErr_SetString(PyExc_TypeError, "Unable to coerce to Emacs object");
    return NULL;
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
        PyObject *args = Py_BuildValue("(NN)",
                                       emacs_object(&EmacsObjectType, symbol),
                                       emacs_object(&EmacsObjectType, data));
        PyErr_SetObject(type, args);
        return NULL;
    }

    return emacs_object(&EmacsObjectType, ret);
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

static bool cmp(emacs_value a, emacs_value b, int op, bool *error)
{
    // Emacs doesn't have a not-equals function, so let's just negate equality
    if (op == Py_NE)
        return !cmp(a, b, Py_EQ, error);

    *error = false;

    // Choose which equality predicate to use based on the types involved. This
    // should make equality behave as close as possible to Python equality.
    if (op == Py_EQ) {
        if (em_numberp(a) && em_numberp(b))
            return em_equal_sign(a, b);
        if (em_stringp(a) && em_stringp(b))
            return em_string_equal(a, b);
        return em_equal(a, b);
    }

    // Strings have their own ordering functions
    if (em_stringp(a) && em_stringp(b)) {
        if (op == Py_LT)
            return em_string_lt(a, b);
        else if (op == Py_LE)
            return !em_string_gt(a, b);
        else if (op == Py_GT)
            return em_string_gt(a, b);
        else if (op == Py_GE)
            return !em_string_lt(a, b);
    }

    // Regular ordering uses number-or-marker-p
    if (em_number_or_marker_p(a) && em_number_or_marker_p(b)) {
        if (op == Py_LT)
            return em_lt(a, b);
        else if (op == Py_LE)
            return em_le(a, b);
        else if (op == Py_GT)
            return em_gt(a, b);
        else if (op == Py_GE)
            return em_ge(a, b);
    }

    *error = true;
    return false;
}

PyObject *EmacsObject_cmp(PyObject *pa, PyObject *pb, int op)
{
    emacs_value a = ((EmacsObject *)pa)->val;
    emacs_value b = NULL;

    if (em_numberp(a) && PyLong_Check(pb)) {
        int overflow;
        long long val = PyLong_AsLongLongAndOverflow(pb, &overflow);
        if (PyErr_Occurred())
            return NULL;
        if (overflow)
            Py_RETURN_FALSE;
        b = em_int(val);
    }
    else if (em_numberp(a) && PyFloat_Check(pb)) {
        double val = PyFloat_AsDouble(pb);
        if (PyErr_Occurred()) return NULL;
        b = em_float(val);
    }
    else if (em_stringp(a) && PyUnicode_Check(pb)) {
        char *val = PyUnicode_AsUTF8AndSize(pb, NULL);
        if (!val) return NULL;
        b = em_str(val);
    }
    else if (!PyObject_TypeCheck(pb, &EmacsObjectType)) {
        if (op == Py_EQ)
            Py_RETURN_FALSE;
        else if (op == Py_NE)
            Py_RETURN_TRUE;
        else {
            PyErr_SetString(PyExc_TypeError, "Unorderable types");
            return NULL;
        }
    }
    else
        b = ((EmacsObject *)pb)->val;

    bool error;
    bool ret = cmp(a, b, op, &error);

    if (error) {
        PyErr_SetString(PyExc_TypeError, "Unorderable types");
        return NULL;
    }
    else if (ret)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject *normalize(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;

    if (em_integerp(val))
        return PyNumber_Long(self);
    else if (em_floatp(val))
        return PyNumber_Float(self);
    else if (em_stringp(val))
        return PyObject_Str(self);
    return NULL;
}

NORMALIZED_BINARY_OPERATION(add, PyNumber_Add)
NORMALIZED_BINARY_OPERATION(subtract, PyNumber_Subtract)
NORMALIZED_BINARY_OPERATION(multiply, PyNumber_Multiply)
NORMALIZED_BINARY_OPERATION(remainder, PyNumber_Remainder)
NORMALIZED_BINARY_OPERATION(divmod, PyNumber_Divmod)

PyObject *EmacsObject_power(PyObject *self, PyObject *other, PyObject *mod)
{
    PyObject *c_self = self, *c_other = other, *c_mod = mod;

    if (PyObject_TypeCheck(self, &EmacsObjectType))
        c_self = normalize(c_self);
    if (PyObject_TypeCheck(other, &EmacsObjectType))
        c_other = normalize(c_other);
    if (PyObject_TypeCheck(mod, &EmacsObjectType))
        c_mod = normalize(c_mod);

    PyObject *ret = NULL;

    if (c_self && c_other && c_mod)
        ret = PyNumber_Power(c_self, c_other, c_mod);
    else
        PyErr_SetString(PyExc_TypeError, "Unsupported operand types");

    if (c_self != self) Py_XDECREF(c_self);
    if (c_other != other) Py_XDECREF(c_other);
    if (c_mod != mod) Py_XDECREF(c_mod);

    return ret;
}

NORMALIZED_UNARY_OPERATION(negative, PyNumber_Negative)
NORMALIZED_UNARY_OPERATION(positive, PyNumber_Positive)
NORMALIZED_UNARY_OPERATION(absolute, PyNumber_Absolute)
NORMALIZED_UNARY_OPERATION(invert, PyNumber_Invert)
NORMALIZED_BINARY_OPERATION(lshift, PyNumber_Lshift)
NORMALIZED_BINARY_OPERATION(rshift, PyNumber_Rshift)
NORMALIZED_BINARY_OPERATION(floordivide, PyNumber_FloorDivide)
NORMALIZED_BINARY_OPERATION(truedivide, PyNumber_TrueDivide)

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

static PyMethodDef EmacsObject_methods[] = {
    METHOD(type, NOARGS),
    METHOD(is_a, VARARGS),
    {NULL},
};

static PyNumberMethods EmacsObject_NumMethods[] = {
    EmacsObject_add,                  // nb_add
    EmacsObject_subtract,             // nb_subtract
    EmacsObject_multiply,             // nb_multiply
    EmacsObject_remainder,            // nb_remainder
    EmacsObject_divmod,               // nb_divmod
    EmacsObject_power,                // nb_power
    EmacsObject_negative,             // nb_negative
    EmacsObject_positive,             // nb_positive
    EmacsObject_absolute,             // nb_absolute
    (inquiry)EmacsObject_bool,        // nb_bool
    EmacsObject_invert,               // nb_invert
    EmacsObject_lshift,               // nb_lshift
    EmacsObject_rshift,               // nb_rshift
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
    EmacsObject_floordivide,          // nb_floor_divide
    EmacsObject_truedivide,           // nb_true_divide
    0,                                // nb_inplace_floor_divide
    0,                                // nb_inplace_true_divide
    0,                                // nb_index
    0,                                // nb_matrix_multiply
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
