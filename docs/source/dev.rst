=======================
Contributing to Tripoli
=======================

For development, it is recommended to install the Python modules in *editable*
form.

.. code:: bash

   pip install --user --editable .

To run Emacs with Tripoli, you can use the :code:`make run` target.

Run the tests with pytest using

.. code:: bash

   make check

or

.. code:: bash

   make check-verbose

Or use the :code:`test` script in the build directory to pass additional
arguments to pytest (such as :code:`-v`).

You can enter a REPL using

.. code:: bash

   make repl

When starting a REPL, the modules :code:`emacs_raw` and :code:`emacs` are
automatically imported as :code:`er` and :code:`e`, respectively.

.. note::
   So far I have not been able to embed IPython or anything more sophisticated than
   the default Python REPL (help appreciated).
