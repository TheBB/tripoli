from collections import namedtuple
from importlib.machinery import ModuleSpec
from itertools import groupby, product
from functools import partial

import emacs_raw


_symbol_value = emacs_raw.intern('symbol-value')
_symbol_function = emacs_raw.intern('symbol-function')
_fboundp = emacs_raw.intern('fboundp')
_boundp = emacs_raw.intern('boundp')
_set = emacs_raw.intern('set')


class EmacsNamespaceFinder:

    def find_spec(self, name, path, target):
        if not name.startswith('emacs'):
            return None
        module = EmacsNamespace()
        for p in name.split('.')[1:]:
            module = getattr(module, p)
        return ModuleSpec(name, EmacsNamespaceLoader(module), origin='emacs')


class EmacsNamespaceLoader:

    def __init__(self, module):
        self.module = module

    def create_module(self, spec):
        return self.module

    def exec_module(self, module):
        pass


class Indexer:

    def __init__(self, type_, args=(), kwargs={}):
        self.type_ = type_
        self.args = args
        self.kwargs = kwargs

    def __repr__(self):
        return self.type_


def syms(convert=True):
    """syms(convert=True)

Returns an iterable of the possible symbols that may be pointed to, in order of
priority.

:param convert: If true, returns Emacs symbols. If false, return strings.

.. code:: python

   some_namespace[syms()]       # => iterable of symbols
"""
    return Indexer('syms', (), {'convert': convert})


def seps(separators):
    """seps(separators)

Returns a new namespace object representing the same symbol(s), but with a
different separator search space.

:param str separators: A string of separator characters.

.. note::
   The new separators are not applied to the existing symbol, only future
   children.

   .. code:: python

      emacs.a.b[seps('~')].c.d  # => a-b~c~d, not a~b~c~d
"""
    return Indexer('seps', (separators,))


clear_cache = Indexer('clear_cache')
"""Child namespaces are cached for performance reasons. Clear the cache by
indexing with this object.

.. code:: python

   some_namespace[clear_cache]
"""


sym = Indexer('sym')
"""Returns the highest priority symbol, whether bound or not.

.. code:: python

   some_namespace[sym]
"""


fbinding = Indexer('fbinding')
"""Returns the function binding of the first symbol with an available function
binding.

:raises NameError: If no such symbol is found

.. code:: python

   some_namespace[fbinding]
"""


binding = Indexer('binding')
"""Returns the variabe binding of the first symbol with an available variabe
binding.

:raises NameError: If no such symbol is found

.. code:: python

   some_namespace[binding]
"""


def fbound(exists=True):
    """fbound(exists=True)

Returns the first symbol with an available function binding.

:param exists: If *True*, raises a *NameError* if no function binding is found.
    If *False*, instead returns the highest priority symbol in that case.

.. code:: python

   some_namespace[fbound()]
"""
    return Indexer('fbound', (), {'exists': exists})


def bound(exists=True):
    """bound(exists=True)

Returns the first symbol with an available variable binding.

:param exists: If *True*, raises a *NameError* if no variable binding is found.
    If *False*, instead returns the highest priority symbol in that case.

.. code:: python

   some_namespace[bound()]
"""
    return Indexer('bound', (), {'exists': exists})


class EmacsNamespace:

    def __init__(self, prefix=None, seps='-/:|'):
        self.__prefix = [] if prefix is None else prefix
        self.__separators = seps
        self.__clear_cache()
        self.__path__ = None

    def __clear_cache(self):
        self.__cached_subs = {}

    def __getitem__(self, item):
        if isinstance(item, str):
            return EmacsNamespace(self.__prefix + [[item]], self.__separators)
        if not isinstance(item, Indexer):
            return None
        if item.type_ == 'clear_cache':
            self.__clear_cache()
            return
        if item.type_ == 'syms':
            return self.__symbols(*item.args, **item.kwargs)
        if item.type_ == 'seps':
            return EmacsNamespace(self.__prefix, *item.args)
        if item.type_ == 'sym':
            return next(self.__symbols())
        if item.type_ == 'fbinding':
            return _symbol_function(self.__function_symbol())
        if item.type_ == 'binding':
            return _symbol_value(self.__variable_symbol())
        if item.type_ == 'fbound':
            return self.__function_symbol(*item.args, **item.kwargs)
        if item.type_ == 'bound':
            return self.__variable_symbol(*item.args, **item.kwargs)

    def __emacs__(self, prefer_symbol=False):
        if prefer_symbol:
            return self[sym]
        return self[binding]

    def __getattr__(self, attr):
        if attr in self.__cached_subs:
            return self.__cached_subs[attr]

        name = attr
        seps = self.__separators
        if self.__prefix:
            finals = {c[-1] for c in self.__prefix[-1]}
            if any(c.isalnum() for c in finals):
                name = '_' + name
            if len(finals) == 1:
                c = next(iter(finals))
                if not c.isalnum():
                    seps = c[-1]

        new_prefix = []
        for is_sep, chars in groupby(name, lambda c: c == '_'):
            if is_sep:
                ilen = sum(1 for _ in chars)
                new_prefix.append([c*ilen for c in seps])
            else:
                new_prefix.append([''.join(chars)])
            seps = self.__separators

        ret = EmacsNamespace(self.__prefix + new_prefix, self.__separators)
        self.__cached_subs[attr] = ret
        return ret

    def __symbols(self, convert=True):
        iters = []
        for parts in product(*self.__prefix[::-1]):
            name = ''.join(parts[::-1])
            if convert: yield emacs_raw.intern(name)
            else: yield name

    def __symbol_satisfying(self, predicate, exists):
        found = None
        for s in self.__symbols():
            if predicate(s):
                if found:
                    raise NameError()
                found = s
        if found is None and exists:
            raise NameError()
        elif found is None:
            sym = next(self.__symbols())
            return sym
        return found

    def __function_symbol(self, exists=True):
        return self.__symbol_satisfying(_fboundp, exists=exists)

    def __variable_symbol(self, exists=True):
        return self.__symbol_satisfying(_boundp, exists=exists)

    def __call__(self, *args, **kwargs):
        func = self.__function_symbol()
        return func(*args, **kwargs)

    def __setattr__(self, name, value):
        if isinstance(value, EmacsNamespace):
            return
        if name.startswith('_EmacsNamespace__') or name == '__path__' or name == '__spec__':
            self.__dict__[name] = value
        else:
            sym = getattr(self, name)[bound(exists=False)]
            _set(sym, value)
