from collections.abc import MutableSequence

from ..util import PlaceOrSymbol, coerce
from emacs import car, cdr, cons, length, setcar, setcdr, symbol_value


class List(PlaceOrSymbol, MutableSequence):
    """Wraps an Emacs list in a Pythonic list interface.

    :param place: The list to wrap. Can be a symbol, an Emacs namespace object,
        a string, an :class:`.EmacsObject` (symbol or otherwise), or `None` (in
        which case the place is assumed to be `nil`).
    """

    def __init__(self, place=None):
        PlaceOrSymbol.__init__(self, place)

    @coerce(only={'value'})
    def push(self, value):
        """Push an element to the front of the list. This is considerably
        faster than :python:`append()`.
        """
        if self.place:
            head = self.place
            tail = cons(car(head), cdr(head))
            setcar(head, value)
            setcdr(head, tail)
        else:
            self.bind(cons(value, self.place))

    @coerce(only={'value'})
    def insert(self, index, value):
        if index == 0:
            self.push(value)
            return
        prev = self.cell(index - 1)
        cell = cdr(prev)
        if cell:
            tail = cons(car(cell), cdr(cell))
            setcar(cell, value)
            setcdr(cell, tail)
        else:
            setcdr(prev, cons(value, None))

    def clear(self):
        self.bind(None)

    def cell(self, index):
        """Get the cons cell associated with a given index."""
        p = self.place
        for _ in range(index):
            p = cdr(p)
            if not p:
                raise IndexError('list index out of range')
        return p

    def cells(self):
        """Iterate over all cons cells in the list."""
        p = self.place
        while p:
            yield p
            p = cdr(p)

    def __bool__(self):
        return bool(self.place)

    def __len__(self):
        return int(length(self.place))

    def __getitem__(self, key):
        return car(self.cell(key))

    @coerce(only={'value'})
    def __setitem__(self, key, value):
        setcar(self.cell(key), value)

    def delete(self, keys):
        keys = sorted(keys)
        p = self.place
        track = 0
        for k in keys:
            k -= track
            if k == 0:
                p = cdr(p)
                track += 1
                self.bind(p)
            else:
                for _ in range(k - 1):
                    p = cdr(p)
                    track += 1
                    if not p:
                        raise IndexError('list index out of range')
                track += 1
                setcdr(p, cdr(cdr(p)))

    def __delitem__(self, key):
        self.delete([key])

    def __iter__(self):
        p = self.place
        while p:
            yield car(p)
            p = cdr(p)
