#include <emacs-module.h>

#ifndef INTERFACE_H
#define INTERFACE_H

/**
 * \file interface.h
 * \brief Defines a more convenient interface to the Emacs runtime.
 */

typedef emacs_value (*em_func) (emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *);


// Environment management

/**
 * \brief Set the Emacs environment.
 *
 * Do not use this function directly, instead use the SET_ENV macro.
 */
void set_environment(emacs_env *env);

/**
 * \brief Return the current Emacs environment.
 */
emacs_env *get_environment();

/**
 * \brief Set the Emacs environment.
 *
 * This macro stores the current environment in a temporary variable, and it can
 * (and MUST) be restored before the functione exist using UNSET_ENV().
 *
 * In any function that receives a new Emacs environment, this macro MUST be
 * called before querying the Emacs runtime (in other words, calling any
 * function in the em_ namespace.)
 */
#define SET_ENV(env) emacs_env *__tmp = get_environment(); set_environment(env)

/**
 * \brief Restore the previous Emacs environment.
 *
 * In any function that receives a new Emacs environment (and, therefore, any
 * function which has called SET_ENV), this macro MUST be called before
 * returning.
 */
#define UNSET_ENV() set_environment(__tmp)


// Convenience functions

/**
 * \brief Provide a feature to Emacs.
 */
void em_provide(char *feature_name);


// Constructors

/**
 * \brief Create an interned symbol.
 */
emacs_value em_intern(char *name);

/**
 * \brief Create a string.
 * \param str UTF-8 encoded string contents (caller keeps ownership).
 */
emacs_value em_str(char *str);

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
emacs_value em_function(em_func func, ptrdiff_t min_nargs, ptrdiff_t max_nargs,
                        const char *doc, void *data);


// Value extraction

/**
 * \brief Extract a symbol name.
 * \param val An Emacs object (must be a symbol).
 * \return UTF-8 encoded name (caller must free).
 */
char *em_symbol_name(emacs_value val);

/**
 * \brief Extract a string.
 * \param val An Emacs object (must be a string).
 * \return UTF-8 encoded string contents (caller must free).
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


// Type checking

/**
 * \brief Return the name of the type as a string.
 * \return UTF-8 encoded type name (caller must free).
 */
char *em_type_as_str(emacs_value val);

/**
 * \brief Check whether an Emacs object has a given type.
 * \param obj The Emacs object to check.
 * \param type The type to compare against.
 */
bool em_type_is(emacs_value val, char *type);


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


// Function calling and error reporting

/**
 * \brief Call a function.
 *
 * If exit_signal is not NULL, exit information is returned.
 *
 * \param func Emacs object to call (must be callable).
 * \param nargs Number of arguments to pass.
 * \param args Argument list.
 * \param exit_signal If not NULL, the exit information is returned here.
 * \param exit_symbol The exit symbol.
 * \param exit_data The exit data.
 * \return Function return value.
 */
emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args,
                       enum emacs_funcall_exit *exit_signal,
                       emacs_value *exit_symbol, emacs_value *exit_data);

/**
 * \brief Call a function, without error checking.
 *
 * \param func Emacs object to call (must be callable).
 * \param nargs Number of arguments to pass.
 * \param args Argument list.
 * \return Function return value.
 */
emacs_value em_funcall_naive(emacs_value func, int nargs, emacs_value *args);

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


// Equality and inequality

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


// Other functions

/**
 * \brief Returns the printed representation of an Emacs object.
 */
char *em_print_obj(emacs_value obj);

#endif /* INTERFACE_H */
