from functools import wraps
from inspect import signature, Parameter
import inspect
from enum import IntEnum

from tripoli.namespace import EmacsNamespace, bound
from emacs_raw import EmacsObject, intern, symbolp


class CoercionStrategy(IntEnum):
    ignore = 0                  # Don't coerce
    coerce = 1                  # Coerce
    coerce_args = 2             # Coerce each element and form tuple
    coerce_kwargs = 3           # Coerce each value and form dict


_setq = intern('set')
_symbol_value = intern('symbol-value')


def coerce(default=True, exceptional=(), symbol=False, symbol_exceptional=()):
    """Decorator for coercing arguments to Emacs objects.

    :param default: If true, all arguments are coerced by default.
    :param exceptional: Arguments for which the default behaviour is inverted,
        i.e. if *default* is true, this is a list of arguments that will *not*
        be coerced.
    :param symbol: If true, string-like arguments will be coerced to symbols if
        possible. If false, they remain strings. Optionally, it may be a string,
        in which case an argument named *self* must be present, and objects
        passed as *self* must have an attribute with the same name as the value
        of `symbol`. The truth value of this attribute then determines the
        symbol-coercing behaviour.
    :param symbol_exceptional: Arguments for which the default meaning of
        *symbol* is inverted.

    This decorator will work with :code:`*args` and :code:`**kwargs` as
    expected, that is, the elements themselves are coerced, not the collections
    as a whole.

    .. note:: Arguments named *self* will never be coerced.
    """
    def decorator(fn):
        # Establish the exceptional and default coercion strategies
        strategies = {name: CoercionStrategy(int(not default)) for name in exceptional}
        default_strategy = CoercionStrategy(int(default))
        symbol_strategies = {name: not symbol for name in symbol_exceptional}

        signature = inspect.signature(fn)

        # If the function signature has splat operators, ensure they are treated correctly
        splat, doublesplat = None, None
        for name, param in signature.parameters.items():
            if param.kind == Parameter.VAR_POSITIONAL and strategies.get(name, default_strategy):
                strategies[name] = CoercionStrategy.coerce_args
            elif param.kind == Parameter.VAR_KEYWORD and strategies.get(name, default_strategy):
                strategies[name] = CoercionStrategy.coerce_kwargs

        # Never coerce self
        strategies['self'] = CoercionStrategy.ignore

        @wraps(fn)
        def ret(*args, **kwargs):
            binding = signature.bind(*args, **kwargs)

            # Check whether to prefer symbols or not
            if isinstance(symbol, str):
                default_symbol = getattr(binding.arguments['self'], symbol)
            else:
                default_symbol = symbol

            # Coerce each argument as necessary
            for key, value in binding.arguments.items():
                strategy = strategies.get(key, default_strategy)
                if not strategy:
                    continue
                prefer_symbol = symbol_strategies.get(key, default_symbol)
                if strategy == CoercionStrategy.coerce:
                    value = EmacsObject(value, prefer_symbol=prefer_symbol)
                elif strategy == CoercionStrategy.coerce_args:
                    value = tuple(EmacsObject(v, prefer_symbol=prefer_symbol) for v in value)
                elif strategy == CoercionStrategy.coerce_kwargs:
                    value = {n: EmacsObject(v, prefer_symbol=prefer_symbol) for n, v in value.items()}
                binding.arguments[key] = value

            return fn(*binding.args, **binding.kwargs)
        return ret
    return decorator


class PlaceOrSymbol:
    """A generic class for wrapping an Emacs value that can either be tied to a
    symbol (representing that symbol's value binding) or, otherwise, any other
    Emacs value.

    This is useful in certain cases. For example, an Emacs list technically
    only a cons cell, but certain mutable operations (deletion, for example)
    may be impossible to perform in-place without reassigning the head of the
    list. That is why functions like Emacs' *delete* both mutates a list *and*
    returns the new head cons cell, and why the Emacs manual recommends
    expressions such as

    .. code:: lisp

       (setq foo (delete element foo))

    to correctly perform deletion.

    :param place: The place to track. May be a symbol, an Emacs namespace
        object or a string (all interpreted as symbols), or any other Emacs
        object or *None*.
    """
    def __init__(self, place=None):
        if place is None:
            self._place = intern('nil')
        elif isinstance(place, EmacsNamespace):
            self._symbol = place[bound(exists=False)]
        elif isinstance(place, str):
            self._symbol = intern(place)
        elif not isinstance(place, EmacsObject):
            raise TypeError('Invalid place')
        elif not symbolp(place):
            self._place = place
        else:
            self._symbol = place

    def __emacs__(self, prefer_symbol=False):
        return self.place

    def __bool__(self):
        return bool(self.place)

    @property
    def place(self):
        """Return the actual Emacs object being tracked. If tracking a symbol,
        returns that symbol's value binding.
        """
        if hasattr(self, '_place'):
            return self._place
        return _symbol_value(self._symbol)

    @coerce()
    def bind(self, value):
        """Rebind to a new object. If tracking a symbol, update that symbol's
        value binding.
        """
        if hasattr(self, '_place'):
            self._place = value
        else:
            _setq(self._symbol, value)
