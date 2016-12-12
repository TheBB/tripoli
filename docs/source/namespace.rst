========================
The :code:`emacs` module
========================

The :code:`emacs` module is not a “real” module, but an object that gives
dynamic access to the global Emacs namespace in a way that mimics genuine Python
modules.


Attribute access
----------------

Accessing attributes on the :code:`emacs` module yields objects that represent
symbols that may or may not have function or value bindings in Emacs.

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

Adjacent separators must always be equal.


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


Import mechanism
----------------

When the :mod:`tripoli` module is imported, an import hook is inserted into
Python’s import mechanism to handle importing the :code:`emacs` module and its
children, so it can be used as a regular module.

.. code:: python

   import emacs.package as package
   from emacs import package


Special methods
---------------

All special attributes and methods end with an underscore. If you need to access
a symbol ending in a separator, please use the item access syntax. All of these
work on the root object as well as child objects, unless specified otherwise.

.. attribute:: emacs.raw_

   This special attribute only works on the root :code:`emacs` object, and it
   allows access to the :mod:`emacs_raw` module.

.. function:: emacs.clear_cache_()

   Child symbols are cached for performance reasons. Clear the cache by calling
   this method.

.. function:: emacs.symbols_(convert=True)

   Iterates over all symbols described by this object. Makes no guarantee that
   the symbols are bound.

   :param convert: If *true*, return Emacs objects. If not, return strings.

.. attribute:: emacs._

   Returns the default symbol, i.e. the first yielded from :func:`symbols_()`.

.. function:: emacs.fs_(exists=True)

   Returns the first symbol in :func:`symbols_()` with a function binding (a
   symbol that satisfies :code:`fboundp`).

   :param exists: If *true*, signal an error if no symbol is found. If *false*,
                  return the default symbol.

.. function:: emacs.vs_(exists=True)

   Returns the first symbol in :func:`symbols_()` with a value binding (a symbol
   that satisfies :code:`boundp`).

   :param exists: If *true*, signal an error if no symbol is found. If *false*,
                  return the default symbol.

.. attribute:: emacs.fb_

   Returns the function binding of the first symbol in :func:`symbols_()` with a
   function binding.

   :raises NameError: if no such symbol is found

.. attribute:: emacs.vb_

   Returns the value binding of the first symbol in :func:`symbols_()` with a
   value binding.

   :raises NameError: if no such symbol is found

.. function:: emacs.seps_(s)

   Return a new namespace object representing the same symbol(s), but with a
   different separator search space.

   :param str s: A string of separator characters.

   .. note::
      The new separators are not applied to the existing symbol, only future
      children.

      .. code:: python

         emacs.a.b.seps_('~').c.d  # => a-b~c~d, not a~b~c~d
