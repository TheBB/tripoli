=================
Launching Tripoli
=================

To load the library from within Emacs, first make sure that the path where the
library is installed is in your :code:`load-path`. Then :code:`(require
'libtripoli)` should suffice.

Once this is done, some functions should be available on the Emacs side.

- :code:`(tripoli-import MODULE)` imports a Python module by name.
- :code:`(tripoli-exec-str CODE)` runs Python code in the form of a string.
- :code:`(tripoli-exec-buffer)` runs the Python code in the current buffer.
- :code:`(tripoli-exec-file FILE)` runs a given Python file.
- :code:`(tripoli-test &rest ARGS)` runs the tests.

When loaded, Tripoli will run one of the files :code:`~/.emacs.py` or
:code:`~/.emacs.d/init.py` if present. You can inhibit this behavior by binding
:code:`tripoli-inhibit-init` to a non-nil value before loading Tripoli.
