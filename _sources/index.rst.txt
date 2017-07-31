=======
Tripoli
=======

This is the documentation for *Tripoli*, a Python-Emacs interface. With it, you
can configure Emacs with Python instead of Emacs lisp.

Reasons why you might want to do such a thing include:

- you simply like Python,
- you don’t like Python but you positively loathe Emacs lisp,
- your existing Emacs configuration is a bit too fast for your taste, or
- you want to see if you can.

.. warning::
   Tripoli is in flux and highly experimental software.


Project overview
================

.. toctree::
   :maxdepth: 2

   Building <building>
   Launching <usage>
   Contributing <dev>


The Design of Tripoli
=====================

Tripoli is organized in three layers, each increasingly Pythonic and less Emacsy.

1. When imported, the library launches an embedded Python interpreter and
   imports a module called :code:`emacs_raw`, which provides a relatively
   minimalistic interface to Emacs. It is designed to provide the bare minimum
   of machinery necessary to build upon in Python. See :doc:`raw`.
2. When the :code:`tripoli` module is imported, a “fake” module named
   :code:`emacs` is available. This module and its submodules are dynamically
   generated when imported, and provide a reasonably intuitive interface to the
   Emacs namespace, making it appear to be modular when in fact it is not. See
   :doc:`namespace`.
3. The third layer is not yet done.

When loaded, Tripoli will run one of the files :code:`~/.emacs.py` or
:code:`~/.emacs.d/init.py` if present. You can inhibit this behavior by binding
:code:`tripoli-inhibit-init` to a non-nil value before loading Tripoli.


API reference
=============

.. toctree::
   :maxdepth: 2

   raw
   namespace


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

