=======================
Contributing to Tripoli
=======================

For development, it is recommended to install the Python modules in *editable*
form.

.. code:: bash

   pip install --user --editable .

To run Emacs with Tripoli, you can use the :code:`make run` target.

Run the tests with pytest using the :code:`make check` target or the :code:`make
check-verbose` target.

You can run Emacs with the built library using the :code:`make run` target or
the :code:`make run-bare` target (which does not run your init).

You can enter a REPL the using :code:`make repl` target. When starting a REPL,
the modules :code:`emacs_raw`, :code:`emacs` and :code:`tripoli` are
automatically imported as :code:`er`, :code:`e` and :code:`t` respectively.

.. note::
   So far I have not been able to embed IPython or anything more sophisticated than
   the default Python REPL (help appreciated).
