from .namespace import EmacsNamespace
import emacs_raw as er


def defun(name):
    name = er.EmacsObject(name, prefer_symbol=True)
    if not er.symbolp(name):
        raise TypeError('Function name must be a symbol')

    def decorator(fn):
        er.intern('fset')(name, er.function(fn))
        return fn
    return decorator
