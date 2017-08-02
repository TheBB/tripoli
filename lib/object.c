#include <Python.h>

#include "emacs-interface.h"
#include "error.h"
#include "module.h"

#include "object.h"



// Construction and destruction

PyObject *EmacsObject__make(PyTypeObject *type, emacs_value val)
{
    EmacsObject *self = (EmacsObject *)type->tp_alloc(type, 0);
    if (self)
        self->val = em_make_global(val);
    return (PyObject *)self;
}

bool EmacsObject__coerce(PyObject *arg, bool prefer_symbol, emacs_value *ret)
{
    if (PyObject_TypeCheck(arg, &EmacsObjectType))
        *ret = ((EmacsObject *)arg)->val;
    else if (PyObject_HasAttrString(arg, "__emacs__")) {
        PyObject *args = PyTuple_New(0);
        PyObject *kwargs = PyDict_New();
        PyDict_SetItemString(kwargs, "prefer_symbol", prefer_symbol ? Py_True : Py_False);

        PyObject *method = PyObject_GetAttrString(arg, "__emacs__");
        PyObject *pyret = PyObject_Call(PyObject_GetAttrString(arg, "__emacs__"), args, kwargs);
        Py_DECREF(args); Py_DECREF(kwargs); Py_DECREF(method);

        if (!pyret || !PyObject_TypeCheck(pyret, &EmacsObjectType)) {
            Py_XDECREF(pyret);
            return false;
        }

        *ret = ((EmacsObject *)pyret)->val;
        Py_DECREF(pyret);
    }
    else if (arg == Py_None || arg == Py_False)
        *ret = em__nil;
    else if (arg == Py_True)
        *ret = em__t;
    else if (PyLong_Check(arg)) {
        int overflow;
        intmax_t val = PyLong_AsLongLongAndOverflow(arg, &overflow);
        if (PyErr_Occurred() || overflow)
            return false;
        *ret = em_int(val);
    }
    else if (PyFloat_Check(arg)) {
        double val = PyFloat_AsDouble(arg);
        if (PyErr_Occurred())
            return false;
        *ret = em_float(val);
    }
    else if (PyUnicode_Check(arg)) {
        char *val = PyUnicode_AsUTF8(arg);
        if (!val)
            return false;
        if (prefer_symbol)
            *ret = em_intern(val);
        else
            *ret = em_str(val);
    }
    else if (PyTuple_Check(arg)) {
        int size = Py_SAFE_DOWNCAST(PyTuple_Size(arg), Py_ssize_t, int);
        emacs_value items[size];
        for (int i = 0; i < size; i++) {
            if (!EmacsObject__coerce(PyTuple_GetItem(arg, i), prefer_symbol, &items[i]))
                return false;
        }
        *ret = em_funcall(em__list, size, items);
    }
    else if (PyList_Check(arg)) {
        int size = Py_SAFE_DOWNCAST(PyList_Size(arg), Py_ssize_t, int);
        emacs_value items[size];
        for (int i = 0; i < size; i++) {
            if (!EmacsObject__coerce(PyList_GetItem(arg, i), prefer_symbol, &items[i]))
                return false;
        }
        *ret = em_funcall(em__list, size, items);
    }
    else if (PyCallable_Check(arg)) {
        PyObject *pydoc = PyObject_GetAttrString(arg, "__doc__");
        char *doc = NULL;
        if (pydoc && PyUnicode_Check(pydoc)) {
            if (!(doc = PyUnicode_AsUTF8(pydoc)))
                PyErr_Clear();
        }

        Py_INCREF(arg);
        *ret = em_function(call_function, 0, emacs_variadic_function, doc, arg);
    }
    else
        return false;

    return true;
}

DOCSTRING(EmacsObject,
          "EmacsObject(obj, prefer_symbol=False, require_symbol=False)\n\n"
          "Creates an Emacs lisp object from `obj`, according to the following rules.\n\n"
          "1. If its argument is an :class:`.EmacsObject` it will wrap the same underlying "
          "Emacs object.\n\n"
          "   .. note::\n"
          "      This creates a *new Python object* that wraps the *same* "
          "Emacs object, however :class:`.EmacsObject` instances are immutable so that "
          "hardly matters.\n\n"
          "2. If the argument has an :code:`__emacs__` magic method, it is called. "
          "This method *must* return an :class:`.EmacsObject` instance, and it must also accept "
          "the `prefer_symbol` keyword argument.\n\n"
          "   .. note::\n"
          "      It is up to the implementation of each class to decide on the "
          "appropriate meaning of `prefer_symbol` in this case.\n\n"
          "3. :code:`None` and :code:`False` become :code:`nil`, "
          "while :code:`True` becomes :code:`t`.\n"
          "4. Integers and floating point numbers become their obvious Emacs lisp counterparts.\n"
          "5. Strings become strings unless `prefer_symbol` is true, in which case they become symbols.\n"
          "6. Lists and tuples become lists, the elements of which are coerced recursively.\n"
          "7. Anything else yields an error.\n\n"
          ".. note::\n"
          "   Since Emacs lisp has so many different structures for mappings, there is no "
          "automatic coercion for dicts.\n\n"
          "If `require_symbol` is true and the coerced object is not a symbol, an error is thrown.")
PyObject *EmacsObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    if (PyTuple_Size(args) == 0)
        return EmacsObject__make(type, em_intern("nil"));
    PyObject *arg;
    int prefer_symbol = false, require_symbol = false;
    char *keywords[] = {"obj", "prefer_symbol", "require_symbol", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|pp", keywords,
                                     &arg, &prefer_symbol, &require_symbol))
        return NULL;
    prefer_symbol |= require_symbol;

    emacs_value coerced;
    if (!EmacsObject__coerce(arg, prefer_symbol, &coerced)) {
        if (!PyErr_Occurred())
            PyErr_SetString(PyExc_TypeError, "Unable to coerce to Emacs object");
        return NULL;
    }

    if (require_symbol && !em_symbolp(coerced)) {
        PyErr_SetString(PyExc_TypeError, "A symbol is required");
        return NULL;
    }

    return EmacsObject__make(type, coerced);
}

void EmacsObject_dealloc(EmacsObject *self)
{
    em_free_global(self->val);
}



// Python Object protocol

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
    Py_ssize_t len = PyTuple_Size(args) + (kwds ? 2 * PyDict_Size(kwds) : 0);
    emacs_value e_arglist[len];

    Py_ssize_t i;
    for (i = 0; i < PyTuple_Size(args); i++) {
        PyObject *arg = PyTuple_GetItem(args, i);
        if (!EmacsObject__coerce(arg, 0, &e_arglist[i])) {
            PyErr_SetString(PyExc_TypeError, "Unable to coerce to Emacs Object");
            return NULL;
        }
    }

    Py_ssize_t ppos = 0;
    PyObject *key, *value;
    while (kwds && PyDict_Next(kwds, &ppos, &key, &value)) {
        char *kw = PyUnicode_AsUTF8(key);
        if (!kw)
            return NULL;

        char *buf = (char *)malloc((strlen(kw) + 2) * sizeof(char));
        sprintf(buf, ":%s", kw);
        for (char *c = buf; *c != '\0'; c++)
            if (*c == '_') *c = '-';
        e_arglist[i++] = em_intern(buf);
        free(buf);

        if (!EmacsObject__coerce(value, 0, &e_arglist[i++])) {
            PyErr_SetString(PyExc_TypeError, "Unable to coerce to Emacs Object");
            return NULL;
        }
    }

    int ilen = Py_SAFE_DOWNCAST(len, Py_ssize_t, int);

    emacs_value func = ((EmacsObject *)self)->val;
    emacs_value ret = em_funcall(func, ilen, e_arglist);

    if (propagate_emacs_error())
        return NULL;

    return EmacsObject__make(&EmacsObjectType, ret);
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
        if (PyErr_Occurred())
            return NULL;
        b = em_float(val);
    }
    else if (em_stringp(a) && PyUnicode_Check(pb)) {
        char *val = PyUnicode_AsUTF8(pb);
        if (!val)
            return NULL;
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
    bool ret = em_compare(a, b, op, &error);

    if (error) {
        PyErr_SetString(PyExc_TypeError, "Unorderable types");
        return NULL;
    }
    else if (ret)
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

Py_ssize_t EmacsObject_Size(PyObject *self)
{
    emacs_value val = ((EmacsObject *)self)->val;
    intmax_t length = 0;

    if (em_arrayp(val)) {
        emacs_value elength = em_funcall_1(em__length, val);
        length = em_extract_int(elength);
    }
    else {
        while (em_consp(val)) {
            length++;
            val = em_funcall_1(em__cdr, val);
        }
        if (em_truthy(val)) {
            PyErr_SetString(PyExc_TypeError, "Improper Emacs sequence");
            return -1;
        }
    }

    return Py_SAFE_DOWNCAST(length, intmax_t, Py_ssize_t);
}

PyObject *EmacsObject_GetItem(PyObject *self, Py_ssize_t i)
{
    emacs_value val = ((EmacsObject *)self)->val;

    if (em_arrayp(val)) {
        if (i >= EmacsObject_Size(self)) {
            PyErr_SetString(PyExc_IndexError, "Index out of bounds");
            return NULL;
        }
        emacs_value obj = em_funcall_2(em__aref, val, em_int(i));
        return EmacsObject__make(&EmacsObjectType, obj);
    }

    while (em_consp(val) && i > 0) {
        i--;
        val = em_funcall_1(em__cdr, val);
    }

    if (i == 0 && em_consp(val)) {
        emacs_value obj = em_funcall_1(em__car, val);
        return EmacsObject__make(&EmacsObjectType, obj);
    }

    if ((i > 0 && em_consp(val)) || (i == 0 && !em_truthy(val))) {
        PyErr_SetString(PyExc_IndexError, "Index out of bounds");
        return NULL;
    }

    PyErr_SetString(PyExc_TypeError, "Improper Emacs sequence");
    return NULL;
}



// Normalized operations

static PyObject *EmacsObject__normalize(PyObject *self)
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

PyObject *EmacsObject_power(PyObject *self, PyObject *other, PyObject *mod)
{
    PyObject *c_self = self, *c_other = other, *c_mod = mod;

    if (PyObject_TypeCheck(self, &EmacsObjectType))
        c_self = EmacsObject__normalize(c_self);
    if (PyObject_TypeCheck(other, &EmacsObjectType))
        c_other = EmacsObject__normalize(c_other);
    if (PyObject_TypeCheck(mod, &EmacsObjectType))
        c_mod = EmacsObject__normalize(c_mod);

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

#define NORMALIZED_BINARY_OPERATION(name, pyname)                               \
    PyObject *EmacsObject_ ## name(PyObject *self, PyObject *other)             \
    {                                                                           \
        PyObject *c_self = self, *c_other = other;                              \
        if (PyObject_TypeCheck(self, &EmacsObjectType))                         \
            c_self = EmacsObject__normalize(c_self);                            \
        if (PyObject_TypeCheck(other, &EmacsObjectType))                        \
            c_other = EmacsObject__normalize(c_other);                          \
                                                                                \
        PyObject *ret = NULL;                                                   \
                                                                                \
        if (c_self && c_other)                                                  \
            ret = pyname(c_self, c_other);                                      \
        else                                                                    \
            PyErr_SetString(PyExc_TypeError, "Unsupported operand types");      \
                                                                                \
        if (c_self != self) Py_XDECREF(c_self);                                 \
        if (c_other != other) Py_XDECREF(c_other);                              \
        return ret;                                                             \
    }

NORMALIZED_BINARY_OPERATION(add, PyNumber_Add)
NORMALIZED_BINARY_OPERATION(subtract, PyNumber_Subtract)
NORMALIZED_BINARY_OPERATION(multiply, PyNumber_Multiply)
NORMALIZED_BINARY_OPERATION(remainder, PyNumber_Remainder)
NORMALIZED_BINARY_OPERATION(divmod, PyNumber_Divmod)
NORMALIZED_BINARY_OPERATION(lshift, PyNumber_Lshift)
NORMALIZED_BINARY_OPERATION(rshift, PyNumber_Rshift)
NORMALIZED_BINARY_OPERATION(floor_divide, PyNumber_FloorDivide)
NORMALIZED_BINARY_OPERATION(true_divide, PyNumber_TrueDivide)

#undef NORMALIZED_BINARY_OPERATION

#define NORMALIZED_UNARY_OPERATION(name, pyname)                                \
    PyObject *EmacsObject_ ## name(PyObject *self)                              \
    {                                                                           \
        PyObject *c_self = self;                                                \
        if (PyObject_TypeCheck(self, &EmacsObjectType))                         \
            c_self = EmacsObject__normalize(c_self);                            \
                                                                                \
        PyObject *ret = NULL;                                                   \
                                                                                \
        if (c_self)                                                             \
            ret = pyname(c_self);                                               \
        else                                                                    \
            PyErr_SetString(PyExc_TypeError, "Unsupported operand types");      \
                                                                                \
        if (c_self != self) Py_XDECREF(c_self);                                 \
        return ret;                                                             \
    }

NORMALIZED_UNARY_OPERATION(negative, PyNumber_Negative)
NORMALIZED_UNARY_OPERATION(positive, PyNumber_Positive)
NORMALIZED_UNARY_OPERATION(absolute, PyNumber_Absolute)
NORMALIZED_UNARY_OPERATION(invert, PyNumber_Invert)

#undef NORMALIZED_UNARY_OPERATION



// Emacs type checking


DOCSTRING(EmacsObject_type,
          "type()\n\n"
          "Return a string representation of the Emacs type of this object.\n\n"
          "Equivalent to :lisp:`(symbol-name (type-of obj))` in elisp.")
PyObject *EmacsObject_type(PyObject *self)
{
    emacs_value obj = ((EmacsObject *)self)->val;
    char *str = em_type_of(obj);
    PyObject *ret = PyUnicode_FromString(str);
    free(str);
    return ret;
}

DOCSTRING(EmacsObject_is_a,
          "is_a(type)\n\n"
          "Check whether the object has the given Emacs type.\n\n"
          "Equivalent to :lisp:`(eq (type-of obj) (intern type))` in elisp.")
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



// Python type object

#define METHOD(name, args)                                              \
    {#name, (PyCFunction)EmacsObject_ ## name, METH_ ## args, __doc_EmacsObject_ ## name}

static PyMethodDef EmacsObject_methods[] = {
    METHOD(type, NOARGS),
    METHOD(is_a, VARARGS),
    {NULL},
};

#undef METHOD

static PyNumberMethods EmacsObject_NumMethods[] = {{
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
    EmacsObject_floor_divide,         // nb_floor_divide
    EmacsObject_true_divide,          // nb_true_divide
    0,                                // nb_inplace_floor_divide
    0,                                // nb_inplace_true_divide
    0,                                // nb_index
    0,                                // nb_matrix_multiply
    0,                                // nb_inplace_matrix_multiply
}};

static PySequenceMethods EmacsObject_SequenceMethods[] = {{
    EmacsObject_Size,                 // sq_length
    0,                                // sq_concat
    0,                                // sq_repeat
    EmacsObject_GetItem,              // sq_item
    0,                                // was_sq_slice
    0,                                // sq_ass_item
    0,                                // was_sq_ass_slice
    0,                                // sq_contains
    0,                                // sq_inplace_concat
    0,                                // sq_inplace_repeat
}};

PyTypeObject EmacsObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "emacs_raw.EmacsObject",          // tp_name
    sizeof(EmacsObject),              // tp_basicsize
    0,                                // tp_itemsize
    (destructor)EmacsObject_dealloc,  // tp_dealloc
    0,                                // tp_print
    0,                                // tp_getattr
    0,                                // tp_setattr
    0,                                // tp_as_async
    EmacsObject_repr,                 // tp_repr
    EmacsObject_NumMethods,           // tp_as_number
    EmacsObject_SequenceMethods,      // tp_as_sequence
    0,                                // tp_as_mapping
    0,                                // tp_hash
    EmacsObject_call,                 // tp_call
    EmacsObject_str,                  // tp_str
    0,                                // tp_getattro
    0,                                // tp_setattro
    0,                                // tp_as_buffer
    Py_TPFLAGS_DEFAULT,               // tp_flags
    __doc_EmacsObject,                // tp_doc
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
