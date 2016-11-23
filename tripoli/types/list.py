from ..util import PlaceOrSymbol, emacsify_args

from emacs import car, cdr, cons, length, setcar, setcdr


class List(PlaceOrSymbol):

    def __init__(self, place=None):
        PlaceOrSymbol.__init__(self, place)

    @emacsify_args(avoid={0})
    def push(self, value):
        if self.place:
            head = self.place
            tail = cons(car(head), cdr(head))
            setcar(head, value)
            setcdr(head, tail)
        else:
            if not self.bindable:
                raise TypeError("Can't push to an empty non-bound list")
            self.bind(cons(value, self.place))

    def cell(self, key):
        p = self.place
        for _ in range(key):
            p = cdr(p)
            if not p:
                raise IndexError('list index out of range')
        return p

    def cells(self):
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

    @emacsify_args(only={2})
    def __setitem__(self, key, value):
        setcar(self.cell(key), value)

    def delete(self, keys):
        keys = sorted(keys)
        p = self.place
        track = 0
        for k in keys:
            k -= track
            if k == 0:
                if not self.bindable:
                    raise TypeError("Can't delete first element of a non-bound list")
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
