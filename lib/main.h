#include "util.h"

#ifndef MAIN_H
#define MAIN_H


int plugin_is_GPL_compatible;

DOCSTRING(exec_buffer,
          "(tripoli-exec-buffer)\n\n"
          "Executes the Python code in the current buffer.")
emacs_value exec_buffer(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(exec_file,
          "(tripoli-exec-file FILENAME)\n\n"
          "Executes the Python code in the file given by FILENAME.")
emacs_value exec_file(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(exec_str,
          "(tripoli-exec-str CODE)\n\n"
          "Executes the Python code in the string CODE.")
emacs_value exec_str(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(import_module,
          "(tripoli-import MODULE)\n\n"
          "Imports the Python module MODULE.")
emacs_value import_module(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);

DOCSTRING(exec_tests,
          "(tripoli-test &rest ARGS)\n\n"
          "Run the Tripoli test suite with arguments ARGS. Returns error code.")
emacs_value exec_tests(emacs_env *env, ptrdiff_t nargs, emacs_value *args, void *data);


#endif /* MAIN_H */
