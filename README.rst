=======
Tripoli
=======

Tripoli is a Python-Emacs interface. With it, you can configure Emacs with
Python instead of Emacs lisp.

Reasons why you might want to do such a thing include:

- you simply like Python.
- you don’t like Python but you positively loathe Emacs lisp.
- your existing Emacs configuration is a bit too fast for your taste.
- you want to see if you can.

At the moment, the focus has been on accessing Emacs lisp from Python, not the
other way around.


Building
========

First, make sure you have the required dependencies:

- Python 3 is required, probably at least Python 3.4.
- You need to have Emacs installed, and it must be compiled with the
  :code:`--with-modules` option. You should see :code:`MODULES` in your
  :code:`system-configuration-features` variable. Note that this feature is
  experimental and usually not enabled by default in distribution packages.
- You also need the Emacs source code available, for the module header file.
- The shared library build system uses CMake. Pip is recommended for installing
  the Python modules.

Then build the extension module.

.. code:: bash

  mkdir build
  cd build
  cmake ..
  make

You probably need to specify that path to your Emacs source code manually by
providing CMake with a :code:`-DCMAKE_INCLUDE_PATH=…` argument, so that the
:code:`emacs-module.h` header file can be found.

.. code:: bash

  cmake -DCMAKE_INCLUDE_PATH=… ..

Once the library is built, you can install it.

.. code:: bash

  make install

To install the Python modules, do

.. code:: bash

  pip install --user .


Usage
=====

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


Development
===========

For development, it is recommended to install the Python modules in *editable*
form.

.. code:: bash

  pip install --user --editable .

If you run :code:`tripoli --dev`, it will try to find :code:`libtripoli.so` in
the build folder instead. (Specifically, it will look for the newest
:code:`libtripoli.so` in any subdirectory in the parent directory of
:code:`tripoli_run/__main__.py`, which should work *if* you install the Python
modules editable.)

Run the tests with pytest using

.. code:: bash

  make check

Additional arguments will be passed to pytest.

You can enter a REPL using

.. code:: bash

  make repl

So far I have not been able to embed IPython or anything more sophisticated than
the default Python REPL (help appreciated). When starting a REPL, the modules
:code:`emacs_raw` and :code:`emacs` are automatically imported as :code:`er` and
:code:`e`, respectively.


Structure
=========

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

**Note**: I would love to Sphinxify this documentation if I can find a way to
run Sphinx from an embedded Python instance and have it read extension modules.
Assistance appreciated!


The :code:`emacs_raw` module
============================


Emacs Objects
-------------

The most important member of :code:`emacs_raw` is the :code:`EmacsObject` type,
which wraps any object in Emacs lisp. All calls into Emacs will return values of
type :code:`EmacsObject`.

As a rule, Emacs object primitives behave like their corresponding Python
primitives. Arithmetic with numbers should work as expected, as well as adding
strings. Every Emacs object except :code:`nil` is truthy when in a boolean
context. All such operations return *Python* types.

**Note**: String indexing and iteration is not yet supported.

Emacs objects support :code:`repr` which is returns essentially the output of
:code:`(format "%S" obj)` in Emacs lisp. The output of `str` is the same, except
when the wrapped object is actually a string, in which case it will be without
the quotes.

The :code:`EmacsObject` constructor tries to coerce its argument to an Emacs
object. It accepts an optional keyword argument :code:`prefer_symbol` (false by
default). It operates according to the following rules:

1. If its argument is an :code:`EmacsObject` it will wrap the same underlying
   Emacs object. (Note, this creates a *new Python object* that wraps the *same*
   Emacs object, however :code:`EmacsObject` instances are immutable so that
   hardly matters.)
2. If the argument has an :code:`__emacs__` magic method, it is called. This
   method *must* return an :code:`EmacsObject`, and it must also accept the
   :code:`prefer_symbol` keyword argument.
3. :code:`None` and :code:`False` become :code:`nil`, while :code:`True` becomes
   :code:`t`.
4. Integers and floating point numbers become their obvious counterparts.
5. Strings become strings unless :code:`prefer_symbol` is true, in which case
   they become interned symbols.
6. Two-element tuples become cons cells. Tuples of other sizes will trigger an
   error. The car and cdr are coerced recursively to  Emacs objects.
7. Lists will become lists, the elements of which are coerced recursively to
   Emacs objects.
8. Anything else triggers an error.

**Note:** No automatic coercion for dicts! Too many different Emacs
implementations of mappings to choose from.

**Note:** Often you *must* have symbols. For this, there’s the
:code:`require_symbol` keyword argument. It implies :code:`prefer_symbol` and
will error if the resulting value is not a symbol.

Emacs objects are also callable, and calling them will work as expected.
Arguments will be automatically coerced to Emacs objects according to the above
rules (:code:`prefer_symbol` set to false). Keyword arguments are appended to
the argument list as follows:

.. code:: python

   obj(1, 'str', kw_arg=1.2)

   # Equivalent to
   # (obj 1 "str" :kw-arg 1.2)


Other functions
---------------

The :code:`emacs_raw` module supports the following typechecking functions,
which behave exactly as their Emacs lisp counterparts:

- :code:`integerp`
- :code:`floatp`
- :code:`numberp`
- :code:`number_or_marker_p`
- :code:`stringp`
- :code:`symbolp`
- :code:`consp`
- :code:`vectorp`
- :code:`listp`
- :code:`functionp`

**Note**: These will return *false* for objects that aren’t Emacs objects, even
ones that are most certainly integers, floats, etc.

Additionally, the following comparison functions are available, which behave
exactly as their Emacs lisp counterparts:

- :code:`eq`
- :code:`eql`
- :code:`equal`
- :code:`equal_sign` (same as :code:`=` in Emacs lisp)
- :code:`string_equal` (same as :code:`string=` in Emacs lisp)
- :code:`lt` (same as :code:`<` in Emacs lisp)
- :code:`le` (same as :code:`<=` in Emacs lisp)
- :code:`gt` (same as :code:`>` in Emacs lisp)
- :code:`ge` (same as :code:`>=` in Emacs lisp)
- :code:`string_lt` (same as :code:`string<` in Emacs lisp)
- :code:`string_gt` (same as :code:`string>` in Emacs lisp)

**Note**: These functions produce *errors* when called with arguments that are
not Emacs objects. Instead, the :code:`EmacsObject` type supports all comparison
operators in a way that is consistent with Python types and their established
behaviour.

Some constructors are available for constructing Emacs objects of specific types:

- :code:`intern` for symbols
- :code:`str` for strings
- :code:`int` for integers
- :code:`float` for floating point numbers
- :code:`function` for functions (this one requires a Python callable, and
  accepts optional keyword arguments :code:`min_nargs` and :code:`max_nargs`;
  note that the Python callable will only receive positional arguments)


Exceptions
----------

Python exceptions and Emacs non-local exits are converted as required across the
interface boundary as they propagate through the call stack.

Every Emacs non-local exit is represented on the Python side by the exception
classes :code:`emacs_raw.Signal` (created by e.g. :code:`signal` or
:code:`error` in Emacs lisp), or :code:`emacs_raw.Throw` (created by
:code:`throw`).

The :code:`args` attribute on these exception objects is a two-element tuple
containing the *symbol* and the *data*. For signals, the symbol is typically
:code:`error` and the data is a list, the first element of which is the error
message. For throws, the symbol is the tag and the data is the value.

These exceptions can be raised from Python code, too, and will be converted to
the proper Emacs non-local exits if possible. However, it is usually sufficient
to throw regular Python exceptions, which are converted to Emacs errors with an
appropriate message.


The Emacs Namespace layer
=========================

Upon importing the `tripoli` module, Python code has access to a “magical”
`emacs` module which provides dynamic access to the Emacs namespace in a way
that is intended to mimic “real” Python modules. This works well as long as the
symbols represented on the Emacs side follow standard Emacs naming.

This is best explained through example:

.. code:: python

   emacs.thing             # => thing
   emacs.some_other_thing  # => some-other-thing
   emacs.some.other.thing  # => some-other-thing
   emacs.private._member   # => private--member

As you can see, attribute access with :code:`.` and underscores behave
identically by inserting separators. This has the fortunate coincidence that a
leading underscore represents private members as expected.

This will also find symbols using non-standard separators, specifically
:code:`-/:|` are searched in order. The search order starts with all hyphens,
and it starts with non-standard separators in the beginning, and not the end, so
it should find your symbol relatively quickly unless it is truly unusual.

Symbol search is lazy, which means that you can import non-existing “modules” as
long as you feel like until you actually need to use them for anything.

If you feel that performance is problematic or you can’t find the symbols you
need, you can access a specific symbol by indexing:

.. code:: python

   emacs['some**weird++symbol']  # => some**weird++symbol
   emacs.prefix['~~~']           # => prefix~~~

Note, no implicit separators are used in this case.

These “modules” support some magical attributes and methods for introspection,
all ending with a single underscore (on the assumption that this is an usual
kind of symbol to find in Emacs):

- :code:`raw_`: Only working on the root `emacs` object, this grants access to
  the `emacs_raw` module.
- :code:`clear_cache_()`: Symbols are cached for performance reasons. This lets
  you clear that cache.
- :code:`_`: Returns the “default” symbol, usually the one with all separators
  hyphens.
- :code:`fs_(exists=True)`: Returns a symbol with an existing function binding.
  If it does not exist, and :code:`exists` is true, an error is thrown.
  Otherwise, the default symbol is returned.
- :code:`vs_(exists=True)`: Like :code:`fs_` except looks for value bindings.
- :code:`fb_`: The function binding of the symbol :code:`fs_()`.
- :code:`fb_`: The value binding of the symbol :code:`vs_()`.
- :code:`symbols_(convert=True)`: An iterator with all the potential symbols
  represented by this object. If :code:`convert`, they will be converted to
  Emacs objects, otherwise just strings.
- :code:`seps_(s)`: Return a new namespace object representing the same Emacs
  symbol(s), but with a different separator search space. Note that the new
  separators are not applied to the existing parts of the symbol, only future
  ones. The argument :code:`s` is a string of separator characters.

Example for :code:`seps_(s)`:

.. code:: python

   emacs.a.b.seps_('~').c.d  # => a-b~c~d

These module objects behave as Python modules in every way, e.g. you can do

.. code:: python

   import emacs.package as package
   from emacs import package

They also support calling and attribute assignment.

.. code:: python

   from emacs import package
   package.archives = …
   package.initialize()

   # This is equivalent to the following in Emacs lisp:
   # (setq package-archives …)
   # (package-initialize)

Emacs namespace modules are coercible to Emacs objects, using :code:`self.vb_`
or :code:`self._` depending on whether symbols are preferred or not.


The “additional goodies” layer
==============================

**Note**: Tripoli is under active development, and this layer most of all.

At the end of the day, it would still be inconvenient to use Tripoli if this was
all there was to it. Python supports a great deal of syntax sugar for common use
cases (e.g. list and dict indexing, iteration) which are difficult to replicate
if the entire interface to Emacs consists of calling functions like :code:`car`,
:code:`cdr` and :code:`cons`.

More crucially, coding in Emacs lisp makes heavy use of *macros*, which are
sadly not available unless you are specifically writing Emacs lisp, and it’s
tricky (although not impossible, perhaps) to create a generic interface to
macros.

As such, Tripoli provides a number of convenience functions, decorators, context
managers and classes to make life easier. (Not so many at the moment, but more
to come.)


Creating symbols
----------------

Symbols are ubiquitous in Emacs lisp, and Python has no primitive symbol
datatype that is distinguishable from strings. Since strings are usually coerced
to strings, you will find that you may need to explicitly create symbols by
calling :code:`emacs_raw.intern` for example.

I prefer binding :code:`_` to this function, and this binding is available in
the :code:`tripoli` module.


Lists and alists
----------------

The :code:`tripoli.types` module exports two classes called :code:`List` and
:code:`AssociationList`. They wrap Emacs objects and roughly behave identically
to Python lists and dicts. The constructor accepts an Emacs object that can
either be the first cons cell of the list or a symbol to which the list is
bound.

Since these types are implemented in Emacs as linked lists, you can expect them
to perform poorly compared to Python lists when used in particular ways. For
example, :code:`append` is expensive, while :code:`push` is cheap (appending to
the beginning).

The :code:`AssociationList` class constructor accepts the keyword arguments:

- :code:`inplace=False`: If set to true, assignment will prefer updating
  existing cons cells over pushing to the front. (A memory/performance
  tradeoff.)
- :code:`assq=False`: If set to true, lookup will be done with :code:`assq` and
  :code:`eq` instead of :code:`assoc` and :code:`equal`.
- :code:`prefer_symbol=False`: If set to true, keys are coerced to symbols when
  possible (usually what you want if :code:`assq` is true).

**Note**: At the moment, these wrapper classes do not support deleting from the
head of the list if it was instantiated with a pointer to the cons cell. For
this reason it is preferable to bind it to a symbol when possible. This is
because of how lists are implemented in Emacs lisp. (Did it ever surprise you
that if you want to delete elements from a list in Emacs lisp, the list is
mutated in place, but you still have to :code:`setq` the result? Same reason.)
This limitation is fixable, and will be fixed, albeit not without some
surprising behaviour in other contexts. Arguably, not being able to delete from
the head is *more* surprising.


Functions
---------

The module :code:`tripoli.decorators` supplies three decorators that make life
easier when working with functions.

- :code:`defun(name)`: Creates an Emacs function and binds it to a symbol.
- :code:`add_hook(hook, append=False, local=False)`: Creates an Emacs function
  and adds it to a hook.
- :code:`eval_after_load(feature)`: Creates an Emacs function and runs it after
  :code:`feature` is loaded.

The latter two decorators accept an optional :code:`name` argument to create a
named function (bound to a symbol).

Additionally, all decorators accept an optional :code:`interactive` argument to
make the resulting function interactive (not particularly useful unless it is
named as well).
