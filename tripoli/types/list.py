from collections.abc import MutableSequence

from tripoli.util import PlaceOrSymbol, coerce
from emacs_raw import intern, cons


_length = intern('length')
_car = intern('car')
_cdr = intern('cdr')
_setcar = intern('setcar')
_setcdr = intern('setcdr')


def _push_head(cell, value):
    tail = cons(_car(cell), _cdr(cell))
    _setcar(cell, value)
    _setcdr(cell, tail)


class List(PlaceOrSymbol, MutableSequence):
    """Wraps an Emacs list in a Pythonic list interface.

    :param place: The list to wrap. See :class:`.PlaceOrSymbol` for more
        information.
    :param prefer_symbol: If true, elements are coerced to symbols when
        possible.
    """

    def __init__(self, initializer=None, bind=None, prefer_symbol=None):
        PlaceOrSymbol.__init__(self, bind)
        self.prefer_symbol = prefer_symbol
        if initializer is not None:
            self.clear()
            try:
                initializer = reversed(initializer)
            except TypeError:
                initializer = list(initializer)[::-1]
            for init in initializer:
                self.push(init)

    @classmethod
    def constructor(cls, **params):
        def mklist(initializer=None):
            return cls(initializer, **params)
        return mklist

    def _cells(self):
        """Iterate over all cons cells in the list."""
        p = self.place
        while p:
            yield p
            p = _cdr(p)

    def _cell(self, index):
        """Retrieve the cons cell at a given index."""
        for cell in self._cells():
            if index == 0:
                return cell
            index -= 1
        raise IndexError('List index out of range')

    def __iter__(self):
        for cell in self._cells():
            yield _car(cell)

    def __getitem__(self, index):
        return _car(self._cell(index))

    def __len__(self):
        return int(_length(self.place))

    @coerce('value', prefer_symbol='prefer_symbol')
    def __setitem__(self, index, value):
        _setcar(self._cell(index), value)

    @coerce('value', prefer_symbol='prefer_symbol')
    def insert(self, index, value):
        """Insert an element at a given index."""
        if index == 0 and self.place:        # Insert before current head
            _push_head(self.place, value)
        elif index == 0 and not self.place:  # Empty, create new head
            self._bind(cons(value))
        else:
            prev = self._cell(index - 1)
            cell = _cdr(prev)
            if cell:                         # Insert before interior cell
                _push_head(cell, value)
            else:                            # Insert after final cell
                _setcdr(prev, cons(value))

    def push(self, value):
        """Push an element to the front of the list. This is considerably
        faster than :python:`append()`, and equivalent to :python:`insert(0, value)`.
        """
        self.insert(0, value)

    def clear(self):
        """Set the list to :lisp:`nil`."""
        self._bind(None)

    def delete(self, indices):
        """Delete elements associated with a list of indices."""
        indices = sorted(indices)

        # We iterate through the list, rewriting it as necessary to skip items
        # that should be deleted. `cell` is the current cons cell, and `track`
        # counts the elements we have skipped over.
        cell = self.place
        track = 0

        for ind in indices:
            # Index of element to be deleted relative to the current cell
            ind -= track

            # Delete the first element. Note that this case only happens for
            # elements at the beginning of the original list.
            if ind == 0:
                cell = _cdr(cell)
                track += 1
                self._bind(cell)

            # Skip to the cell BEFORE the one we want to delete, then set its cdr.
            else:
                for _ in range(ind - 1):
                    cell = _cdr(cell)
                    track += 1
                if not cell:
                    raise IndexError('List index out of range')
                track += 1
                _setcdr(cell, _cdr(_cdr(cell)))

    def __delitem__(self, index):
        self.delete([index])
