=====================
The Design of Tripoli
=====================

Tripoli is organized in three layers, each increasingly Pythonic and less Emacsy.

1. When imported, the library launches an embedded Python interpreter and
   imports a module called :code:`emacs_raw`, which provides a relatively
   minimalistic interface to Emacs. It is designed to provide the bare minimum
   of machinery necessary to build upon in Python.
2. When the :code:`tripoli` module is imported, a “fake” module named
   :code:`emacs` is available. This module and its submodules are dynamically
   generated when imported, and provide a reasonably intuitive interface to the
   Emacs namespace, making it appear to be modular when in fact it is not.
3. Additional tools are provided by the :code:`tripoli` module for things that
   are still wanted. This includes classes for standard Emacs data structures,
   as well as decorators and context managers for Emacs lisp macros (which are
   otherwise not very useful from Python).

The design of each of these layers is expanded on in the following.
