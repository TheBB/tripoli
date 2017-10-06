from collections import OrderedDict
from collections.abc import MutableMapping

from tripoli.util import PlaceOrSymbol, coerce
from emacs_raw import eq, cons, intern


_nil = intern('nil')
_car = intern('car')
_cdr = intern('cdr')
_setcdr = intern('setcdr')
_keywordp = intern('keywordp')
_listp = intern('listp')


def _colonify(key):
    if _keywordp(key):
        return key
    key = str(key)
    if key.startswith(':'):
        return intern(key)
    return intern(':' + key)


def _copy_list(cell, endp=lambda c: not c):
    head, tail = None, None
    while cell and not endp(cell):
        if head is None:
            head = cons(_car(cell))
            tail = head
        else:
            new_tail = cons(_car(cell))
            _setcdr(tail, new_tail)
            tail = new_tail
        cell = _cdr(cell)
    return (head or _nil), tail, cell


def _copy_value(cell):
    head, _, _ = _copy_list(_cdr(cell), lambda c: _keywordp(_car(c)))
    return head


class MPList(PlaceOrSymbol, MutableMapping):

    def __init__(self, initializer=None, bind=None, prefer_symbol=False, consistent=False):
        PlaceOrSymbol.__init__(self, bind)
        self.prefer_symbol = prefer_symbol
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
        def mkmplist(*args, **kwargs):
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
        return mkmplist

    def _cells(self):
        cell = self.place
        while cell:
            if _keywordp(_car(cell)):
                yield cell
            cell = _cdr(cell)

    def _cell(self, key, cell=None):
        key = _colonify(key)
        if cell is None:
            cell = self.place
        prev = None
        while cell and not eq(key, _car(cell)):
            prev, cell = cell, _cdr(cell)
        if not cell:
            raise KeyError("Key '{}' not in mplist".format(key))
        return prev, cell

    def __getitem__(self, key):
        _, cell = self._cell(key)
        return _copy_value(cell)

    def __iter__(self):
        for cell in self._cells():
            yield _car(cell)

    def values(self):
        for cell in self._cells():
            yield _copy_value(cell)

    def __len__(self):
        return sum(1 for _ in self._cells())

    def clear(self):
        self._bind(None)

    def __delitem__(self, key):
        prev, cell = self._cell(key)
        while cell:
            following = _cdr(cell)
            while following and not _keywordp(_car(following)):
                following = _cdr(following)
            # print('prev', prev, 'cell', cell, 'following', following)
            if prev:
                _setcdr(prev, following)
            else:
                self._bind(following)

            if self.consistent:
                return

            try:
                prev, cell = self._cell(key, prev)
            except KeyError:
                return

    @coerce('value', prefer_symbol='prefer_symbol')
    def __setitem__(self, key, value):
        key = _colonify(key)
        if not _listp(value):
            raise TypeError('Values must be lists')
        head, tail, _ = _copy_list(value)

        if self.consistent:
            try:
                prev, cell = self._cell(key)
            except KeyError:
                pass
            else:
                cont = _cdr(cell)
                while cont and not _keywordp(_car(cont)):
                    cont = _cdr(cont)
                if head:
                    _setcdr(cell, head)
                    _setcdr(tail, cont)
                else:
                    _setcdr(cell, cont)
                return

        if head:
            _setcdr(tail, self.place)
            self._bind(cons(key, head))
        else:
            self._bind(cons(key, self.place))
