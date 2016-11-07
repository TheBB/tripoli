#include <emacs-module.h>

#ifndef INTERFACE_H
#define INTERFACE_H

/**
 * \file interface.h
 * \brief Defines a more convenient interface to the Emacs runtime.
 */

typedef emacs_value (*em_func) (emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *);


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
 * \brief Check whether an Emacs object is nil.
 */
bool em_null(emacs_value val);

/**
 * \brief Check whether an Emacs object is a list (cons or nil).
 */
bool em_listp(emacs_value val);


// Other functions

/**
 * \brief Set the Emacs environment.
 *
 * This function must be called whenever a new environment is available, before
 * any other `em_`-functions can be used.
 */
void set_environment(emacs_env *env);

/**
 * \brief Call a function.
 * \brief func Emacs object to call (must be callable).
 * \brief nargs Number of arguments to pass.
 * \brief args Argument list.
 * \return Function return value.
 */
emacs_value em_funcall(emacs_value func, int nargs, emacs_value *args);

/**
 * \brief Returns the printed representation of an Emacs object.
 */
char *em_print_obj(emacs_value obj);

#endif /* INTERFACE_H */
