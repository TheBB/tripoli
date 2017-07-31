#include <Python.h>

#include "emacs-interface.h"
#include "error.h"
#include "object.h"
#include "util.h"

#include "module.h"



// Callback

emacs_value call_function(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data)
{
    push_env(env);

    PyObject *function = (PyObject *)data;
    PyObject *arglist = PyTuple_New(nargs);
    for (int i = 0; i < nargs; i++) {
        PyObject *arg = EmacsObject__make(&EmacsObjectType, args[i]);
        PyTuple_SetItem(arglist, i, arg);
    }

    PyObject *py_ret = PyObject_CallObject(function, arglist);
    Py_DECREF(arglist);

    if (propagate_python_error())
        POP_ENV_AND_RETURN(NULL);

    emacs_value ret;
    if (!EmacsObject__coerce(py_ret, 0, &ret)) {
        em_error("Function failed to return a valid Emacs object");
        POP_ENV_AND_RETURN(NULL);
    }

    POP_ENV_AND_RETURN(ret);
}



// Constructors

DOCSTRING(py_intern,
          "intern(name)\n\n"
          "Creates an :class:`.EmacsObject` corresponding to the interned "
          "symbol with the given name. "
          "Equivalent to :lisp:`(intern name)` in elisp.")
PyObject *py_intern(PyObject *self, PyObject *args)
{
    UNUSED(self);
    char *name;
    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;
    return EmacsObject__make(&EmacsObjectType, em_intern(name));
}

DOCSTRING(py_str,
          "str(s)\n\n"
          "Creates an :class:`.EmacsObject` of string type.")
PyObject *py_str(PyObject *self, PyObject *args)
{
    UNUSED(self);
    PyObject *arg, *pystr;
    char *cstr;
    if (!PyArg_ParseTuple(args, "O", &arg))
        return NULL;
    if (!(pystr = PyObject_Str(arg)))
        return NULL;
    cstr = PyUnicode_AsUTF8(pystr);
    Py_DECREF(pystr);
    if (!cstr)
        return NULL;
    return EmacsObject__make(&EmacsObjectType, em_str(cstr));
}

DOCSTRING(py_int,
          "int(i)\n\n"
          "Creates an :class:`.EmacsObject` of integer type.")
PyObject *py_int(PyObject *self, PyObject *args)
{
    UNUSED(self);
    PyObject *arg, *pyint;
    if (!PyArg_ParseTuple(args, "O", &arg))
        return NULL;
    if (!(pyint = PyNumber_Long(arg)))
        return NULL;
    int overflow;
    intmax_t val = PyLong_AsLongAndOverflow(pyint, &overflow);
    Py_DECREF(pyint);
    if (PyErr_Occurred() || overflow)
        return NULL;
    return EmacsObject__make(&EmacsObjectType, em_int(val));
}

DOCSTRING(py_float,
          "float(f)\n\n"
          "Creates an :class:`.EmacsObject` of floating point type.")
PyObject *py_float(PyObject *self, PyObject *args)
{
    UNUSED(self);
    PyObject *arg, *pyfloat;
    if (!PyArg_ParseTuple(args, "O", &arg))
        return NULL;
    if (!(pyfloat = PyNumber_Float(arg)))
        return NULL;
    double val = PyFloat_AsDouble(pyfloat);
    Py_DECREF(pyfloat);
    if (PyErr_Occurred())
        return NULL;
    return EmacsObject__make(&EmacsObjectType, em_float(val));
}

DOCSTRING(py_function,
          "function(callback, nargs_min=0, nargs_max=PTRDIFF_MAX)\n\n"
          "Creates an Emacs function object with the specificed arity which, when called, "
          "will run the given callback function and return its value to the Emacs caller, "
          "provided the return value is coercible to an :class:`.EmacsObject`.\n\n"
          "If exceptions of type :class:`.Signal` or :class:`.Throw` are raised with "
          "appropriate arguments, the corresponding effect will be propagated to Emacs. "
          "All other exceptions are propagated as error signals.")
PyObject *py_function(PyObject *self, PyObject *args, PyObject *kwds)
{
    UNUSED(self);
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
    if (pydoc && PyUnicode_Check(pydoc) && !(doc = PyUnicode_AsUTF8(pydoc)))
        PyErr_Clear();

    Py_XINCREF(fcn);
    emacs_value func = em_function(call_function, min_nargs, max_nargs, doc, fcn);
    return EmacsObject__make(&EmacsObjectType, func);
}

DOCSTRING(py_cons,
          "cons(car=None, cdr=None)\n\n"
          "Creates an :class:`.EmacsObject` of cons type. "
          "If the car is not given, the return value is nil. The cdr defaults to nil.")
PyObject *py_cons(PyObject *self, PyObject *args)
{
    UNUSED(self);
    PyObject *car = Py_None, *cdr = Py_None;
    if (!PyArg_ParseTuple(args, "|O!O!", &EmacsObjectType, &car, &EmacsObjectType, &cdr))
        return NULL;

    if (car == Py_None)
        return EmacsObject__make(&EmacsObjectType, em__nil);

    emacs_value ecar = ((EmacsObject *)car)->val;

    if (cdr == Py_None)
        return EmacsObject__make(&EmacsObjectType, em_cons(ecar, em__nil));

    emacs_value ecdr = ((EmacsObject *)cdr)->val;
    return EmacsObject__make(&EmacsObjectType, em_cons(ecar, ecdr));
}


DOCSTRING(py_list,
          "list(iterable)\n\n"
          "Creates an :class:`.EmacsObject` of list (cons) type. "
          "All elements of the iterable must be Emacs objects.")
PyObject *py_list(PyObject *self, PyObject *args)
{
    UNUSED(self);
    if (PyTuple_Size(args) == 0)
        return EmacsObject__make(&EmacsObjectType, em__nil);

    PyObject *arg;
    if (!PyArg_ParseTuple(args, "O", &arg))
        return NULL;

    PyObject *iterator = PyObject_GetIter(arg);
    if (!iterator)
        return NULL;

    emacs_value head = NULL, tail = NULL;
    PyObject *item;
    while ((item = PyIter_Next(iterator))) {
        if (!PyObject_TypeCheck(item, &EmacsObjectType)) {
            PyErr_SetString(PyExc_TypeError, "Expected EmacsObject");
            Py_DECREF(iterator);
            return NULL;
        }
        if (!head) {
            head = em_cons(((EmacsObject *)item)->val, em__nil);
            tail = head;
        }
        else {
            emacs_value new_tail = em_cons(((EmacsObject *)item)->val, em__nil);
            em_setcdr(tail, new_tail);
            tail = new_tail;
        }

        Py_DECREF(item);
    }

    Py_DECREF(iterator);

    if (!head)
        head = em__nil;
    return EmacsObject__make(&EmacsObjectType, head);
}


DOCSTRING(py_vector,
          "vector(iterable)\n\n"
          "Creates an :class:`.EmacsObject` of vector type. "
          "All elements of the iterable must be Emacs objects.")
PyObject *py_vector(PyObject *self, PyObject *args)
{
    UNUSED(self);

    size_t max_args = 100, nargs = 0;
    emacs_value *eargs = (emacs_value *)malloc(max_args * sizeof(emacs_value));

    if (PyTuple_Size(args) != 0) {
        PyObject *arg;
        if (!PyArg_ParseTuple(args, "O", &arg))
            return NULL;

        PyObject *iterator = PyObject_GetIter(arg);
        if (!iterator)
            return NULL;
        PyObject *item;
        while ((item = PyIter_Next(iterator))) {
            if (!PyObject_TypeCheck(item, &EmacsObjectType)) {
                PyErr_SetString(PyExc_TypeError, "Expected EmacsObject");
                Py_DECREF(iterator);
                free(eargs);
                return NULL;
            }
            eargs[nargs++] = ((EmacsObject *)item)->val;
            if (nargs == max_args) {
                max_args *= 2;
                eargs = (emacs_value *)realloc(eargs, max_args * sizeof(emacs_value));
            }

            Py_DECREF(item);
        }

        Py_DECREF(iterator);
    }

    emacs_value vector = em_funcall(em__vector, nargs, eargs);
    free(eargs);
    return EmacsObject__make(&EmacsObjectType, vector);
}



// Comparison predicates

#define COMPARE(pred)                                                   \
    PyObject *py_ ## pred(PyObject *self, PyObject *args)               \
    {                                                                   \
        UNUSED(self);                                                   \
        PyObject *pa, *pb;                                              \
        if (!PyArg_ParseTuple(args, "O!O!",                             \
                              &EmacsObjectType, &pa,                    \
                              &EmacsObjectType, &pb))                   \
            return NULL;                                                \
        emacs_value a, b;                                               \
        a = ((EmacsObject *)pa)->val;                                   \
        b = ((EmacsObject *)pb)->val;                                   \
        if (em_ ## pred(a, b))                                          \
            Py_RETURN_TRUE;                                             \
        Py_RETURN_FALSE;                                                \
    }

DOCSTRING(py_eq,
          "eq(a, b)\n\n"
          "Check object equality using :lisp:`eq`. Equivalent to :lisp:`(eq a b)` in elisp.")
COMPARE(eq)

DOCSTRING(py_eql,
          "eql(a, b)\n\n"
          "Check object equality using :lisp:`eql`. Equivalent to :lisp:`(eql a b)` in elisp.")
COMPARE(eql)

DOCSTRING(py_equal,
          "equal(a, b)\n\n"
          "Check object equality using :lisp:`equal`. Equivalent to :lisp:`(equal a b)` in elisp.")
COMPARE(equal)

DOCSTRING(py_equal_sign,
          "equal_sign(a, b)\n\n"
          "Check object equality using :lisp:`=`. Equivalent to :lisp:`(= a b)` in elisp.")
COMPARE(equal_sign)

DOCSTRING(py_string_equal,
          "string_equal(a, b)\n\n"
          "Check object equality using :lisp:`string-equal`. "
          "Equivalent to :lisp:`(string-equal a b)` in elisp.")
COMPARE(string_equal)

DOCSTRING(py_lt,
          "lt(a, b)\n\n"
          "Equivalent to :lisp:`(< a b)` in elisp.")
COMPARE(lt)

DOCSTRING(py_le,
          "le(a, b)\n\n"
          "Equivalent to :lisp:`(<= a b)` in elisp.")
COMPARE(le)

DOCSTRING(py_gt,
          "gt(a, b)\n\n"
          "Equivalent to :lisp:`(> a b)` in elisp.")
COMPARE(gt)

DOCSTRING(py_ge,
          "ge(a, b)\n\n"
          "Equivalent to :lisp:`(>= a b)` in elisp.")
COMPARE(ge)

DOCSTRING(py_string_lt,
          "string_lt(a, b)\n\n"
          "Equivalent to :lisp:`(string< a b)` in elisp.")
COMPARE(string_lt)

DOCSTRING(py_string_gt,
          "string_gt(a, b)\n\n"
          "Equivalent to :lisp:`(string> a b)` in elisp.")
COMPARE(string_gt)

#undef COMPARE



// Type predicates

#define TYPECHECK(name)                                         \
    PyObject *py_ ## name(PyObject *self, PyObject *args)       \
    {                                                           \
        UNUSED(self);                                           \
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

DOCSTRING(py_integerp,
          "Check whether the object is an Emacs integer. "
          "Equivalent to :lisp:`(integerp obj)` in elisp.")
TYPECHECK(integerp)

DOCSTRING(py_floatp,
          "floatp(obj)\n\n"
          "Check whether the object is an Emacs float. "
          "Equivalent to :lisp:`(floatp obj)` in elisp.")
TYPECHECK(floatp)

DOCSTRING(py_numberp,
          "numberp(obj)\n\n"
          "Check whether the object is an Emacs number (integer or float). "
          "Equivalent to :lisp:`(numberp obj)` in elisp.")
TYPECHECK(numberp)

DOCSTRING(py_number_or_marker_p,
          "number_or_marker_p(obj)\n\n"
          "Check whether the object is an Emacs number or a marker. "
          "Equivalent to :lisp:`(number-or-marker-p obj)` in elisp.")
TYPECHECK(number_or_marker_p)

DOCSTRING(py_stringp,
          "stringp(obj)\n\n"
          "Check whether the object is an Emacs string. "
          "Equivalent to :lisp:`(stringp obj)` in elisp.")
TYPECHECK(stringp)

DOCSTRING(py_symbolp,
          "symbolp(obj)\n\n"
          "Check whether the object is an Emacs symbol. "
          "Equivalent to :lisp:`(symbolp obj)` in elisp.")
TYPECHECK(symbolp)

DOCSTRING(py_consp,
          "consp(obj)\n\n"
          "Check whether the object is an Emacs cons cell. "
          "Equivalent to :lisp:`(consp obj)` in elisp.")
TYPECHECK(consp)

DOCSTRING(py_vectorp,
          "vectorp(obj)\n\n"
          "Check whether the object is an Emacs vector. "
          "Equivalent to :lisp:`(vectorp obj)` in elisp.")
TYPECHECK(vectorp)

DOCSTRING(py_listp,
          "listp(obj)\n\n"
          "Check whether the object is an Emacs list. "
          "Equivalent to :lisp:`(listp obj)` in elisp.")
TYPECHECK(listp)

DOCSTRING(py_functionp,
          "functionp(obj)\n\n"
          "Check whether the object is an Emacs function. "
          "Equivalent to :lisp:`(functionp obj)` in elisp.")
TYPECHECK(functionp)

#undef TYPECHECK



// Python module initialization

#define METHOD(name, args)                                              \
    {#name, (PyCFunction) py_ ## name, (args), __doc_py_ ## name}

PyMethodDef methods[] = {
    METHOD(intern, METH_VARARGS),
    METHOD(str, METH_VARARGS),
    METHOD(int, METH_VARARGS),
    METHOD(float, METH_VARARGS),
    METHOD(function, METH_VARARGS | METH_KEYWORDS),
    METHOD(cons, METH_VARARGS),
    METHOD(list, METH_VARARGS),
    METHOD(vector, METH_VARARGS),
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

#undef METHOD

PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "emacs_raw", NULL, -1, methods, NULL, NULL, NULL, NULL,
};

DOCSTRING(EmacsSignal,
          "Signal(symbol, data)\n\n"
          "Represents an Emacs signal (errors, mostly). "
          "When raised from Emacs, will have two arguments, a symbol denoting the type of signal "
          "(usually `error`, but not always), and a list with `data`, usually a single string "
          "giving an error message.\n\n")
PyObject *EmacsSignal = NULL;

DOCSTRING(EmacsThrow,
          "Throw(tag, value)\n\n"
          "Represents an Emacs throw (using the :lisp:`throw` built-in). "
          "When raised from Emacs, will have two arguments, a symbol denoting the `tag` "
          "and an arbitrary object denoting the `value`.")
PyObject *EmacsThrow = NULL;

PyObject *PyInit_emacs_raw()
{
    PyObject *mod = PyModule_Create(&module);
    if (!mod)
        return NULL;

    if (PyType_Ready(&EmacsObjectType) < 0)
        return NULL;
    Py_INCREF(&EmacsObjectType);
    PyModule_AddObject(mod, "EmacsObject", (PyObject *)&EmacsObjectType);

    EmacsSignal = PyErr_NewExceptionWithDoc("emacs_raw.Signal", __doc_EmacsSignal, NULL, NULL);
    Py_INCREF(EmacsSignal);
    PyModule_AddObject(mod, "Signal", EmacsSignal);

    EmacsThrow = PyErr_NewExceptionWithDoc("emacs_raw.Throw", __doc_EmacsThrow, NULL, NULL);
    Py_INCREF(EmacsThrow);
    PyModule_AddObject(mod, "Throw", EmacsThrow);

    return mod;
}
