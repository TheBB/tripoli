from collections.abc import MutableMapping

from ..util import coerce
from .list import List
import emacs as e
from emacs import car, caar, cdr, cons, equal, length, setcdr


class AssociationList(MutableMapping, List):
    """Wraps an Emacs alist in a Pythonic mapping interface.

    :param place: The alist to wrap. Can be a symbol, an Emacs namespace
        object, a string, an :class:`.EmacsObject` (symbol or otherwise), or
        `None` (in which case the place is assumed to be `nil`).
    :param bool inplace: Set to true to force updating existing cons cells
        whenver possible, instead of pushing to the head of the list. This
        may improve memory footprint at the cost of a performance penalty to
        :python:`__setitem__`.
    :param bool assq: Set to true to lookup with :lisp:`assq` and compare keys
        with :lisp:`eq` rather than :lisp:`assoc` and :lisp:`equal`,
        respectively. Use this if your keys are symbols.
    :param bool prefer_symbol: Set to true to automatically coerce keys to
        symbols rather than strings. If `assq` is true, you probably want this.
    """

    def __init__(self, place=None, inplace=False, assq=False, prefer_symbol=False):
        List.__init__(self, place)
        self.inplace = inplace
        self.prefer_symbol = prefer_symbol
        if assq:
            self.lookup, self.equal = e.assq, e.eq
        else:
            self.lookup, self.equal = e.assoc, e.equal

    @coerce(prefer_symbol_from_self={'key'})
    def __getitem__(self, key):
        cell = self.lookup(key, self.place)
        if not cell:
            raise KeyError(str(key))
        return cdr(cell)

    @coerce(prefer_symbol_from_self={'key'})
    def __setitem__(self, key, value):
        if self.inplace:
            cell = self.lookup(key, self.place)
            if cell:
                setcdr(cell, value)
                return
        self.push(cons(key, value))

    @coerce(prefer_symbol_from_self={'key'})
    def __delitem__(self, key):
        if self.equal(car(self.place), key) and not self.bindable:
            raise TypeError("Can't delete first element of a non-bound list")
        indices = [i for i, v in enumerate(self.cells()) if self.equal(caar(v), key)]
        self.delete(indices)

    def unique_cells(self):
        """Iterate over all cons cells with unique keys."""
        found = []
        for cell in self.cells():
            key = caar(cell)
            if any(self.equal(c, key) for c in found):
                continue
            found.append(key)
            yield car(cell)

    def __iter__(self):
        for cell in self.unique_cells():
            yield car(cell)

    def __len__(self):
        return sum(1 for _ in self)

    def keys(self):
        return iter(self)

    def values(self):
        for cell in self.unique_cells():
            yield cdr(cell)

    def items(self):
        for cell in self.unique_cells():
            yield car(cell), cdr(cell)
