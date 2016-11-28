from .namespace import EmacsNamespace
import emacs_raw as er


def defun(obj):
    if callable(obj) and not isinstance(obj, EmacsNamespace):
        return er.function(obj)

    if isinstance(obj, EmacsNamespace):
        obj = obj.fs_(exists=False)
    else:
        obj = EmacsObject(obj, prefer_symbol=True)
    assert er.symbolp(obj)

    def decorator(fn):
        efn = er.function(fn)
        er.fset(obj, efn)
        return fn
    return decorator
