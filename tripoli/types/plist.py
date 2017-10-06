from collections import OrderedDict
from collections.abc import MutableMapping

from tripoli.util import PlaceOrSymbol, coerce
from emacs_raw import intern, cons, EmacsObject, eq


_car = intern('car')
_cdr = intern('cdr')
_cadr = intern('cadr')
_cddr = intern('cddr')
_setcar = intern('setcar')
_setcdr = intern('setcdr')
_keywordp = intern('keywordp')


def _colonify(key):
    if _keywordp(key):
        return key
    key = str(key)
    if key.startswith(':'):
        return intern(key)
    return intern(':' + key)


class PList(PlaceOrSymbol, MutableMapping):
    """Wraps an Emacs property list in a Pythonic mapping interface.

    :param place: The property list to wrap. See :class:`.PlaceOrSymbol` for
        more information.
    :param colonify: If true, keys are modified to start with colons.
    :param consistent: If true, internal consistency is enforced (i.e. no
        duplicate keys). Incurs an application-specific runtime expense.
    :param prefer_symbol: If true, elements are coerced to symbols when
        possible.
    """

    def __init__(self, initializer=None, bind=None, colonify=False,
                 prefer_symbol=False, consistent=False):
        PlaceOrSymbol.__init__(self, bind)
        self.prefer_symbol = prefer_symbol
        self.colonify = colonify
        self.consistent = consistent
        if initializer is not None:
            self.clear()
            if hasattr(initializer, 'items'):
                initializer = initializer.items()
            elif hasattr(initializer, 'keys'):
                initializer = ((key, initializer[key]) for key in initializer.keys())
            try:
                initializer = reversed(initializer)
            except TypeError:
                initializer = list(initializer)[::-1]
            for key, value in initializer:
                self[key] = value

    @classmethod
    def constructor(cls, **params):
        def mkplist(*args, **kwargs):
            initializer = None
            if args or kwargs:
                initializer = OrderedDict()
                if args:
                    initializer.update(args[0])
                initializer.update({
                    key.replace('_', '-'): value
                    for key, value in kwargs.items()
                })
            return cls(initializer, **params)
        return mkplist

    def _cells(self):
        """Iterate over every key cell in the plist (that is, every second cons
        cell).
        """
        cell = self.place
        while cell:
            yield cell
            cell = _cddr(cell)

    @coerce('key', prefer_symbol=True)
    def _cell(self, key, cell=None):
        """Given a key, return the two key cells *prev* and *cell*, where
        *cell* is the first key cell corresponding to the *key*, and prev is
        the preceding key cell. *prev* may be *None*.

        :param key: The key to search for.
        :param cell: If given, search starts at this key cell.
        """
        if self.colonify:
            key = _colonify(key)
        if cell is None:
            cell = self.place
        prev = None
        while cell:
            if eq(_car(cell), key):
                return prev, cell
            prev, cell = cell, _cddr(cell)
        raise KeyError("Key '{}' not in plist".format(key))

    def __iter__(self):
        for cell in self._cells():
            yield _car(cell)

    def values(self):
        for cell in self._cells():
            yield _cadr(cell)

    def items(self):
        for cell in self._cells():
            yield (_car(cell), _cadr(cell))

    def __getitem__(self, key):
        _, cell = self._cell(key)
        return _cadr(cell)

    def __len__(self):
        return sum(1 for _ in self._cells())

    def clear(self):
        self._bind(None)

    def __delitem__(self, key):
        prev, cell = self._cell(key)
        while cell:
            if prev:            # The previous cell exists, change its cdr
                _setcdr(_cdr(prev), _cddr(cell))
            else:               # Reset the head of the list
                self._bind(_cddr(cell))

            # If the underlying plist is consistent, we don't need to look for
            # other entries with the same key
            if self.consistent:
                return

            # Loop to find the rest of the keys
            try:
                prev, cell = self._cell(key, prev)
            except KeyError:
                return

    @coerce('value', prefer_symbol='prefer_symbol')
    def __setitem__(self, key, value):
        # If the underlying plist must remain consistent, check to see if the
        # key already exists. If it does, change it.
        if self.consistent:
            try:
                _, cell = self._cell(key)
                _setcar(_cdr(cell), value)
                return
            except KeyError:
                pass

        if self.colonify:
            key = _colonify(key)
        else:
            key = EmacsObject(key, prefer_symbol=True)

        # Push to the head of the list
        head = cons(value, self.place)
        head = cons(key, head)
        self._bind(head)
