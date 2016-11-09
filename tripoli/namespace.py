import emacs
from enum import Enum
from itertools import groupby, product


class Namespace:

    def __init__(self, prefix=None, v=True, f=True, seps='-/:|'):
        assert v or f
        self.__prefix = [] if prefix is None else prefix
        self.__v = v
        self.__f = f
        self.__separators = seps
        self.__clear_cache()

    def __inspect(self):
        test = ''.join(p if p is not None else ' ' for p in self.__prefix)
        return '{}{} {}'.format('v' if self.__v else '', 'f' if self.__f else '', test)

    def __sub(self, **kwargs):
        new_kwargs = {
            'prefix': self.__prefix,
            'v': self.__v,
            'f': self.__f,
            'seps': self.__separators,
        }
        new_kwargs.update(kwargs)
        return Namespace(**new_kwargs)

    def __getitem__(self, index):
        if isinstance(index, str):
            return self.__sub(prefix=self.__prefix + [[index]])

    def __clear_cache(self):
        self.__cached_symbol = None
        self.__cached_subs = {}

    def __getattr_special(self, name):
        if name == 'clear_cache_':
            return self.__clear_cache
        elif name == 'v_':
            return self.__sub(v=True, f=False)
        elif name == 'f_':
            return self.__sub(v=False, f=True)
        elif name == 's_':
            return self.__symbol
        elif name == 'seps_':
            return lambda s: self.__sub(seps=s)
        elif name == 'symbols_':
            return self.__symbols

    def __getattr__(self, name):
        if name.endswith('_'):
            return self.__getattr_special(name)
        if name in self.__cached_subs:
            return self.__cached_subs[name]

        tname = name
        seps = self.__separators
        if self.__prefix:
            finals = {c[-1] for c in self.__prefix[-1]}
            if any(c.isalnum() for c in finals):
                tname = '_' + tname
            if len(finals) == 1:
                c = next(iter(finals))
                if not c.isalnum():
                    seps = c[-1]

        new_prefix = []
        for is_sep, chars in groupby(tname, lambda c: c == '_'):
            if is_sep:
                ilen = sum(1 for _ in chars)
                new_prefix.append([c*ilen for c in seps])
            else:
                new_prefix.append([''.join(chars)])
            seps = self.__separators

        ret = self.__sub(prefix=self.__prefix + new_prefix)
        self.__cached_subs[name] = ret
        return ret

    def __symbols(self, convert=True):
        iters = []
        for parts in product(*self.__prefix[::-1]):
            name = ''.join(parts[::-1])
            if convert: yield emacs.intern(name)
            else: yield name

    @property
    def __symbol(self):
        if self.__cached_symbol is not None:
            return self.__cached_symbol
        predicates = []
        if self.__v: predicates.append(emacs.intern('boundp'))
        if self.__f: predicates.append(emacs.intern('fboundp'))
        for symbol in self.__symbols():
            if any(pred(symbol) for pred in predicates):
                if self.__cached_symbol is not None:
                    raise KeyError('Ambiguous name: {} and {}'.format(
                        symbol, self.__cached_symbol
                    ))
                self.__cached_symbol = symbol
        if self.__cached_symbol is None:
            raise NameError("Couldn't find an appropriate bound symbol")
        return self.__cached_symbol
