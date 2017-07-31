============================
The :code:`emacs_raw` module
============================

.. automodule:: emacs_raw

The :code:`emacs_raw` module is implemented in C and provides a minimal
interface for interfacing with Emacs.

Emacs objects
=============

.. role:: lisp(code)
   :language: lisp
.. role:: python(code)
   :language: python

.. autoclass:: emacs_raw.EmacsObject
   :members:

   Objects of type :class:`.EmacsObject` wrap any kind of Emacs lisp object. As
   a rule, primitives behave like their corresponding Python primitives.
   Arithmetic with numbers should work as expected, as well as adding strings.
   Every Emacs object except :code:`nil` is truthy when in a boolean context.

   .. note::
      All such operations return the expected `Python` types, not new
      `EmacsObject` objects.

   .. note::
      Some behaviour is not yet supported, including string indexing, iterators
      (e.g. over the characters in a string, or elements of a list), or adding
      lists.

   The `repr` of an :class:`.EmacsObject` is typically a string that can be
   passed to Emacs lisp `read` to yield an equivalent object. The `str` of an
   :class:`.EmacsObject` is the same, except without quotes in case the wrapped
   object actually is a string.

   Emacs objects are also callable, and behave as expected if the wrapped object
   is a function. Arguments are automatically coerced to Emacs objects according
   to the above rules (with `prefer_symbol` false). Keyword arguments are also
   supported as follows:

   .. code:: python

      obj(1, 'str', kw_arg=1.2)  # => (obj 1 "str" :kw-arg 1.2)


Custom constructors
===================

.. automodule:: emacs_raw
   :noindex:
   :members: intern, str, int, float, function


Exceptions
==========

.. automodule:: emacs_raw
   :noindex:
   :members: Signal, Throw


Type checking
=============

.. warning::
   The following functions always return false for non-Emacs objects, even
   Python objects that behave as those types. Consider whether EAFP is for you.

.. automodule:: emacs_raw
   :noindex:
   :members: stringp, integerp, floatp, numberp, number_or_marker_p,
             symbolp, consp, vectorp, functionp


Comparison
==========

.. warning::
   The following functions *fail* for non-Emacs objects. Instead you should
   probably use the comparison operators directly, which is supported by the
   :class:`.EmacsObject` type to behave in a way that is consistent with Python
   types and their established behaviour.

.. automodule:: emacs_raw
   :noindex:
   :members: eq, eql, equal, equal_sign, string_equal, lt, le, gt, ge,
             string_lt, string_gt
