#include "util.h"

#ifndef MAIN_H
#define MAIN_H

// Emacs complains unless this symbol exists
int plugin_is_GPL_compatible;

DOCSTRING(import_module,
          "(tripoli-import MODULE)\n\n"
          "Imports the Python module MODULE via Tripoli.\n\n"
          "Returns t on success, nil on failure.");
static emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

/**
 * \brief Initialize the Tripoli library.
 *
 * This starts the embedded Python interpreter, enables the emacs module for it,
 * creates the tripoli-import function in the Emacs runtime and provides the
 * libtripoli feature symbol.
 */
int emacs_module_init(struct emacs_runtime *ert);

#endif /* MAIN_H */
