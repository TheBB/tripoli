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


def add_hook(hook, append=False, local=False, name=None):
    hook = er.EmacsObject(hook, prefer_symbol=True)
    if not er.symbolp(hook):
        raise TypeError('Hook name must be a symbol')
    if name:
        name = er.EmacsObject(name, prefer_symbol=True)
        if not er.symbolp(name):
            raise TypeError('Name must be a symbol')

    def decorator(fn):
        if name:
            defun(name)(fn)
            efn = name
        else:
            efn = er.function(fn)
        er.intern('add-hook')(
            hook, efn,
            er.EmacsObject(append),
            er.EmacsObject(local)
        )
        return fn
    return decorator


def eval_after_load(feature):
    feature = er.EmacsObject(feature)

    def decorator(fn):
        er.intern('eval-after-load')(feature, er.function(fn))
        return fn
    return decorator
