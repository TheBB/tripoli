#include "util.h"

#ifndef MAIN_H
#define MAIN_H

// Emacs complains unless this symbol exists
int plugin_is_GPL_compatible;

DOCSTRING(import_module,
          "(tripoli-import MODULE)\n\n"
          "Imports the Python module MODULE via Tripoli.");
static emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(run_string,
          "(tripoli-run-string CODE)\n\n"
          "Runs the python code in the string CODE.");
static emacs_value run_string(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(run_file,
          "(tripoli-run-file FILENAME)\n\n"
          "Runs the python code in the file given by FILENAME.");
static emacs_value run_file(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(run_buffer,
          "(tripoli-run-buffer)\n\n"
          "Runs the python code in the current buffer.");
static emacs_value run_buffer(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

/**
 * \brief Initialize the Tripoli library.
 *
 * This starts the embedded Python interpreter, enables the emacs module for it,
 * creates the tripoli-import function in the Emacs runtime and provides the
 * libtripoli feature symbol.
 */
int emacs_module_init(struct emacs_runtime *ert);

#endif /* MAIN_H */
