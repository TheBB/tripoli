from collections.abc import MutableMapping

from ..util import emacsify_args
from .list import List
import emacs as e
from emacs import car, caar, cdr, cons, equal, length, setcdr


class AssociationList(MutableMapping, List):

    def __init__(self, place=None, inplace=False, assq=False, prefer_symbol=False):
        List.__init__(self, place)
        self.inplace = inplace
        self.prefer_symbol = prefer_symbol
        if assq:
            self.lookup, self.equal = e.assq, e.eq
        else:
            self.lookup, self.equal = e.assoc, e.equal

    @emacsify_args(prefer_symbol_from_self={1})
    def __getitem__(self, key):
        cell = self.lookup(key, self.place)
        if not cell:
            raise KeyError(str(key))
        return cdr(cell)

    @emacsify_args(prefer_symbol_from_self={1})
    def __setitem__(self, key, value):
        if self.inplace:
            cell = self.lookup(key, self.place)
            if cell:
                setcdr(cell, value)
                return
        self.push(cons(key, value))

    @emacsify_args(prefer_symbol_from_self={1})
    def __delitem__(self, key):
        if self.equal(car(self.place), key) and not self.bindable:
            raise TypeError("Can't delete first element of a non-bound list")
        indices = [i for i, v in enumerate(self.cells()) if self.equal(caar(v), key)]
        self.delete(indices)

    def unique_cells(self):
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
