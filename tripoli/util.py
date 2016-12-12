import inspect
from functools import wraps

from emacs_raw import EmacsObject
import emacs_raw as er

from .namespace import EmacsNamespace
from emacs_raw import intern, symbolp
from emacs import cons, list as mklist, symbol_value, set as setq


def coerce(only=None, avoid=set(), prefer_symbol=False, prefer_symbol_from_self=False):
    """Decorator for coercing arguments to Emacs objects.

    :param only: Only the arguments in this set will be coerced. If set to
        `None`, all arguments are coerced.
    :type only: set or None
    :param set avoid: Arguments in this set will not be coerced, whatever the
        value of `only`.
    :param prefer_symbol: Arguments for which symbols should be preferred over
        strings. If true, applies to all arguments.
    :type prefer_symbol: set or bool
    :param prefer_symbol_from_self: If given, should be a set of arguments for
        which symbols are preferred over strings `if any only if` the value of
        :python:`self.prefer_symbol` is true. This will automatically add
        `self` to the list of avoided arguments. The argument `self` *must* be
        present and it *must* have this attribute.
    :type prefer_symbol_from_self: set or bool
    """
    if prefer_symbol_from_self:
        avoid = avoid | {'self'}
        if isinstance(prefer_symbol_from_self, set) and not prefer_symbol:
            prefer_symbol = set()
    def decorator(fn):
        signature = inspect.signature(fn)
        @wraps(fn)
        def wrap(*args, **kwargs):
            binding = signature.bind(*args, **kwargs)
            psym = prefer_symbol
            if prefer_symbol_from_self and binding.arguments['self'].prefer_symbol:
                psym = psym | prefer_symbol_from_self
            for key, value in binding.arguments.items():
                if key in avoid:
                    continue
                if only is None or key in only:
                    sym = psym if isinstance(psym, bool) else key in psym
                    binding.arguments[key] = EmacsObject(value, prefer_symbol=sym)
            return fn(*binding.args, **binding.kwargs)
        return wrap
    return decorator


def symbolify(only=None, avoid=set()):
    """Decorator for coercing arguments to Emacs symbols.

    Equivalent to :func:`.coerce` with `prefer_symbol` set to true.
    """
    return coerce(only=only, avoid=avoid, prefer_symbol=True)


class PlaceOrSymbol:
    """A generic class for wrapping an Emacs value that can either be tied to a
    `symbol` (representing its value binding) or a `place` (any mutable object).

    For example, an Emacs list is technically only a cons cell, but certain
    mutable operations (deletion, for example) may not work as expected unless
    the value binding of a symbol can be changed to point to a new place.

    :param place: The place to track. Can be a symbol, an Emacs namespace
        object, a string, an :class:`.EmacsObject` (symbol or otherwise), or
        `None` (in which case the place is assumed to be `nil`).
    """

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

    def __emacs__(self, prefer_symbol=False):
        return self.place

    @property
    def place(self):
        """The `place` for this object. If tracking a symbol, equivalent to that
        symbol's value binding."""
        if hasattr(self, '_place'):
            return self._place
        else:
            return symbol_value(self._symbol)

    def bind(self, value):
        """Bind to a new place. If tracking a symbol, updates that symbol's value
        binding."""
        if hasattr(self, '_symbol'):
            setq(self._symbol, value)
        else:
            self._place = value
