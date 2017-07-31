=================
Launching Tripoli
=================

To load the library from within Emacs, first make sure that the path where the
library is installed is in your :code:`load-path`. Then :code:`(require
'libtripoli)` should suffice.

Once this is done, four functions should be available on the Emacs side.

- :code:`(tripoli-import MODULE)` imports a Python module by name.
- :code:`(tripoli-run-string CODE)` runs Python code in the form of a string.
- :code:`(tripoli-run-buffer)` runs the Python code in the current buffer.
- :code:`(tripoli-run-file FILE)` runs a given Python file.

The last two are interactive.

To speed things up, Tripoli ships with a command line executable :code:`tripoli`
that automatically sets up the load path and requires :code:`libtripoli` for
you. This assumes that :code:`libtripoli.so` can be found in one of the usual
locations (:code:`/usr/lib`, :code:`/usr/local/lib` or :code:`/lib`). If that is
not the case you can set the :code:`TRIPOLI_PATH` environment variable.

Additionally, the :code:`tripoli` executable will run one of the files
:code:`~/.emacs.py` or :code:`~/.emacs.d/init.py` if present. This repository
includes a sample :code:`dotemacs.py` that serves as a proof of concept.

The :code:`tripoli` executable passes additional arguments on to Emacs. Note
that this means it will run your regular Emacs init if you do not specify
:code:`-q`.

.. warning::
   Tripoli doesn’t read Emacs arguments, which means that :code:`-q` or
   :code:`-Q` will currently not prevent Tripoli from reading a Python init
   file.
