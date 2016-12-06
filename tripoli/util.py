from emacs_raw import EmacsObject
import emacs_raw as er

from .namespace import EmacsNamespace
from emacs_raw import intern, symbolp
from emacs import cons, list as mklist, symbol_value, set as setq


def emacsify_args(only=None, avoid=set(), prefer_symbol=set(),
                  prefer_symbol_from_self=set()):
    if prefer_symbol_from_self:
        avoid = avoid | {0}
    def local_emacsify(key, value, prefer_symbol):
        if key in avoid:
            return value
        if only is None or key in only:
            sym = (prefer_symbol is True) or (key in prefer_symbol)
            return EmacsObject(value, prefer_symbol=sym)
        return value
    def decorator(fn):
        def ret(*args, **kwargs):
            psym = prefer_symbol
            if prefer_symbol_from_self and args[0].prefer_symbol:
                psym |= prefer_symbol_from_self
            args = [local_emacsify(i, v, psym) for i, v in enumerate(args)]
            kwargs = {k: local_emacsify(k, v, psym) for k, v in kwargs.items()}
            return fn(*args, **kwargs)
        return ret
    return decorator


def symbolify_args(only=None, avoid=set()):
    return emacsify_args(only=only, avoid=avoid, prefer_symbol=True)


class PlaceOrSymbol:

    def __init__(self, place=None):
        if place is None:
            self._place = intern('nil')
        elif isinstance(place, EmacsNamespace):
            self._symbol = place.vs_()
        elif isinstance(place, str):
            self._symbol = er.intern(place)
        elif not isinstance(place, EmacsObject):
            raise TypeError('Invalid place')
        elif not symbolp(place):
            self._place = place
        else:
            self._symbol = place

    @property
    def place(self):
        if hasattr(self, '_place'):
            return self._place
        else:
            return symbol_value(self._symbol)

    @property
    def bindable(self):
        return hasattr(self, '_symbol')

    def bind(self, value):
        if hasattr(self, '_symbol'):
            setq(self._symbol, value)
