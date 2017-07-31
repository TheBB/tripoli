========================
The :code:`emacs` module
========================

The :code:`emacs` module is not a “real” module, but an object that gives
dynamic access to the global Emacs namespace in a way that mimics genuine Python
modules.


Attribute access
----------------

Accessing attributes on the :code:`emacs` module yields objects that represent
delayed computation of symbols that may or may not have function or value
bindings in Emacs.

This is best explained through example:

.. code:: python

   emacs.thing             # => thing
   emacs.some_other_thing  # => some-other-thing
   emacs.some.other.thing  # => some-other-thing
   emacs.private._member   # => private--member

As you can see, attribute access with :code:`.` and underscores behave
identically by inserting hyphens. This has the fortunate coincidence that a
leading underscore represents private members as expected.

These modules also support attribute assignment and method calling.

.. code:: python

   from emacs import package
   package.archives = …
   package.initialize()

   # The above is equivalent to the following in Emacs lisp:
   # (setq package-archives …)
   # (package-initialize)


Non-standard separators
-----------------------

Although most Emacs authors use only hyphens as a separator in symbols, many do
not. Tripoli can find any symbol using separators from the set :code:`-/:|`,
prioritized in that order (i.e. hyphens over forward slashes, and so on).
The search starts at the first separator in the symbol, so that e.g.
:code:`emacs.a.b.c` searches through :code:`a-b-c`, :code:`a/b-c`, :code:`a:b-c`
and :code:`a|b-c` before it tries :code:`a-b/c`.

Symbol search is lazy. You do not have to worry about performance until a symbol
is actually required.

Adjacent separators must always be equal. That is, :code:`emacs.private._member`
will not find a symbol named :code:`private-/member`.


Item access
-----------

If you feel that performance is problematic or you can’t find the symbols you
need, you can access a specific symbol by indexing:

.. code:: python

   emacs['some**weird++symbol']  # => some**weird++symbol
   emacs.prefix['~~~']           # => prefix~~~

Note, no implicit separators are used in this case.


:class:`.EmacsObject` coercion
------------------------------

Emacs namespace objects are automatically coerced to Emacs objects when needed,
preferring the value binding of the symbol, or the symbol itself if
*prefer_symbol* is true. See :class:`.EmacsObject` for more information.

.. note::
   Due to limitations with “Python magic” this coercion does not work when
   assigning to another namespace object. That is, the following does not work
   as expected.

   .. code:: python

      some_namespace.member = some_other_namespace

   Instead you can use (see below)

   .. code:: python

      some_namespace.member = some_other_namespace[binding]

.. note::
   The following does not work either. It simply rebinds the Python name
   :code:`some_namespace`.

   .. code:: python

      some_namespace = some_emacs_object


Import mechanism
----------------

When the :mod:`tripoli` module is imported, an import hook is inserted into
Python’s import mechanism to handle importing the :code:`emacs` module and its
children, so it can be used as a regular module.

.. code:: python

   import emacs.package as package
   from emacs import package


Special indexing
----------------

The :code:`tripoli.namespace` module exports a number of special objects that
can be used to manipulate the behaviour of the dynamic modules more precisely.
This is done via indexing. See the examples below.

.. automodule:: tripoli.namespace
   :noindex:
   :members: syms, seps, clear_cache, sym, fbinding, binding, fbound, bound
