#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

/**
 * \brief Check for Python errors having been thrown, and create Emacs errors.
 *
 * At any time, this function may be called to check whether the Python error
 * indicator is set. If it is, the error is converted to an Emacs non-local exit
 * signal as appropriate, and the Python error indicator is cleared.
 *
 * \return True if an error was present, false otherwise.
 */
bool propagate_python_error();

/**
 * \brief Check for Emacs errors having been thrown, and create Python errors.
 *
 * At any time, this function may be called to check whether the Emacs error
 * indicator is set. If it is, the error is converted to a Python exception
 * as appropriate, and the Emacs error indicator is cleared.
 */
bool propagate_emacs_error();

#endif /* ERROR_H */
