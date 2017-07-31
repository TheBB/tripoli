#include <emacs-module.h>

#ifndef EMACS_INTERFACE_H
#define EMACS_INTERFACE_H



// Internally used and persistent objects

emacs_value em__nil, em__t, em__error, em__eval;
emacs_value em__cons, em__setcar, em__setcdr, em__vector, em__car, em__cdr;
emacs_value em__length, em__aref, em__arrayp;
emacs_value em__format, em__list, em__symbol_name, em__type_of;
emacs_value em__integerp, em__floatp, em__numberp, em__stringp, em__symbolp,
    em__consp, em__vectorp, em__listp, em__functionp, em__number_or_marker_p;
emacs_value em__eq, em__eql, em__equal, em__equal_sign, em__string_equal,
    em__lt, em__le, em__gt, em__ge, em__string_lt, em__string_gt;

/**
 * \brief Populate the internally used objects.
 *
 * This function must be called after push_env() but before using any of the
 * other functions in the em_ namespace. It is only necessary to call it once.
 */
void populate();



// Environment stack

/**
 * \brief A node in a stack of Emacs environments.
 */
typedef struct EnvCons {
    emacs_env *env;
    struct EnvCons *next;
} EnvCons;

/**
 * \brief Sets the current Emacs environment.
 *
 * This macro pushes the current environment on the top of the stack, and it can
 * (and MUST) be restored before the functione exist using pop_env().
 *
 * In any function that receives a new Emacs environment, this function MUST be
 * called before querying the Emacs runtime (in other words, calling any
 * function in the em_ namespace.)
 */
void push_env(emacs_env *env);

/**
 * \brief Gets the current Emacs environment.
 */
emacs_env *get_env();

/**
 * \brief Pops the current Emacs environment.
 *
 * In any function that receives a new Emacs environment (and, therefore, any
 * function which has called push_env), this function MUST be called before
 * returning.
 */
emacs_env *pop_env();

/**
 * \brief Pops the current Emacs environment and returns.
 */
#define POP_ENV_AND_RETURN(val) \
    do { emacs_value __ret = val; pop_env(); return __ret; } while (0)



// Global references

/**
 * \brief Wraps an Emacs value in a global reference.
 *
 * Necessary for maintaining persistent values whose lifetime extends past that of an Emacs environment.
 */
emacs_value em_make_global(emacs_value val);

/**
 * \brief Frees a global reference.
 */
void em_free_global(emacs_value val);



// Basic Emacs types

/**
 * \brief Create an interned symbol.
 */
emacs_value em_intern(const char *name);

/**
 * \brief Create a string.
 * \param str UTF-8 encoded string contents (caller keeps ownership).
 */
emacs_value em_str(const char *str);

/**
 * \brief Create an integer.
 */
emacs_value em_int(intmax_t val);

/**
 * \brief Create a float.
 */
emacs_value em_float(double val);

/**
 * \brief Create a function.
 * \param func The C callback.
 * \param min_nargs Minimum number of arguments.
 * \param max_nargs Maximum number of arguments.
 * \param doc Documentation string.
 * \param data Data pointer.
 */
emacs_value em_function(emacs_subr func, ptrdiff_t min_nargs, ptrdiff_t max_nargs,
                        const char *doc, void *data);

/**
 * \brief Create a cons cell.
 */
emacs_value em_cons(emacs_value car, emacs_value cdr);

/**
 * \brief Extract a symbol name.
 * \param val An Emacs object (must be a symbol).
 * \return UTF-8 encoded name (caller receives ownership).
 */
char *em_symbol_name(emacs_value val);

/**
 * \brief Extract a string.
 * \param val An Emacs object (must be a string).
 * \return UTF-8 encoded string contents (caller receives ownership).
 */
char *em_extract_str(emacs_value val);

/**
 * \brief Extract an integer.
 * \param val An Emacs object (must be an integer).
 */
intmax_t em_extract_int(emacs_value val);

/**
 * \brief Extract a float.
 * \param val An Emacs object (must be a float).
 */
double em_extract_float(emacs_value val);



// Emacs function calling interface

/**
 * \brief Call a function, without error checking.
 *
 * \param func Emacs object to call (must be callable).
 * \param nargs Number of arguments to pass.
 * \param args Argument list.
 * \return Function return value.
 */
emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args);

/**
 * \brief Call a function with no arguments.
 */
emacs_value em_funcall_0(emacs_value func);

/**
 * \brief Call a function with one argument.
 */
emacs_value em_funcall_1(emacs_value func, emacs_value arg);

/**
 * Call a function with two arguments.
 */
emacs_value em_funcall_2(emacs_value func, emacs_value arg1, emacs_value arg2);

/**
 * Call a function with two arguments.
 */
emacs_value em_funcall_3(emacs_value func, emacs_value arg1, emacs_value arg2, emacs_value arg3);



// Predicates

/**
 * \brief Check whether an Emacs object is truthy (not nil).
 */
bool em_truthy(emacs_value val);

/**
 * \brief Check whether an Emacs object is an integer.
 */
bool em_integerp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a float.
 */
bool em_floatp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a number.
 */
bool em_numberp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a number or a marker.
 */
bool em_number_or_marker_p(emacs_value val);

/**
 * \brief Check whether an Emacs object is a string.
 */
bool em_stringp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a symbol.
 */
bool em_symbolp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a cons cell.
 */
bool em_consp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a vector.
 */
bool em_vectorp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a list (cons cell or nil).
 */
bool em_listp(emacs_value val);

/**
 * \brief Check whether an Emacs object is a function (callable).
 */
bool em_functionp(emacs_value val);

/**
 * \brief Check whether an Emacs object is an array.
 */
bool em_arrayp(emacs_value val);

/**
 * \brief Check object identity with eq.
 */
bool em_eq(emacs_value a, emacs_value b);

/**
 * \brief Check object identity with eql (compare by value for floats).
 */
bool em_eql(emacs_value a, emacs_value b);

/**
 * \brief Check object equality with equal (structural equality).
 */
bool em_equal(emacs_value a, emacs_value b);

/**
 * \brief Check object equality with = (numerical equality).
 */
bool em_equal_sign(emacs_value a, emacs_value b);

/**
 * \brief Check string equality.
 */
bool em_string_equal(emacs_value a, emacs_value b);

/**
 * \brief Less than.
 */
bool em_lt(emacs_value a, emacs_value b);

/**
 * \brief Less than or equal to.
 */
bool em_le(emacs_value a, emacs_value b);

/**
 * \brief Greater than.
 */
bool em_gt(emacs_value a, emacs_value b);

/**
 * \brief Greater than or equal to.
 */
bool em_ge(emacs_value a, emacs_value b);

/**
 * \brief String lexicographic ordering.
 */
bool em_string_lt(emacs_value a, emacs_value b);

/**
 * \brief String lexicographic ordering.
 */
bool em_string_gt(emacs_value a, emacs_value b);

/**
 * \brief Compares two Emacs values according to Python semantics.
 *
 * \param a The first object
 * \param b The second object
 * \param op The operation to perform
 * \param error Pointer to error flag
 * \return True if the comparison is truthy, false otherwise.
 */
bool em_compare(emacs_value a, emacs_value b, int op, bool *error);


// Non-local exits

/**
 * \brief Signal a non-local exit.
 */
void em_signal(emacs_value symbol, emacs_value data);

/**
 * \brief Throw a non-local exit.
 */
void em_throw(emacs_value symbol, emacs_value data);

/**
 * \brief Signal an error.
 */
void em_error(char *message);



// Miscellaneous functions

char *em_print_obj(emacs_value val);

/**
 * \brief Return the name of the type as a string.
 * \return UTF-8 encoded type name (caller receives ownership).
 */
char *em_type_of(emacs_value val);

/**
 * \brief Check whether an Emacs object has a given type.
 * \param obj The Emacs object to check.
 * \param type The type to compare against.
 */
bool em_type_is(emacs_value val, const char *type);

void em_setcar(emacs_value cons, emacs_value car);
void em_setcdr(emacs_value cons, emacs_value cdr);
void em_provide(const char *feature_name);
void em_defun(emacs_subr func, const char *name,
              ptrdiff_t min_nargs, ptrdiff_t max_nargs,
              bool interactive, emacs_value interactive_spec,
              const char *doc, void *data);


#endif /* EMACS-INTERFACE_H */
