from .namespace import EmacsNamespace
import emacs_raw as er


def defun(name):
    name = er.EmacsObject(name, require_symbol=True)

    def decorator(fn):
        er.intern('fset')(name, er.function(fn))
        return fn
    return decorator


def add_hook(hook, append=False, local=False, name=None):
    hook = er.EmacsObject(hook, require_symbol=True)
    if name:
        name = er.EmacsObject(name, require_symbol=True)

    def decorator(fn):
        if name:
            defun(name)(fn)
            efn = name
        else:
            efn = er.function(fn)
        er.intern('add-hook')(hook, efn, append, local)
        return fn
    return decorator


def eval_after_load(feature):
    feature = er.EmacsObject(feature)

    def decorator(fn):
        er.intern('eval-after-load')(feature, er.function(fn))
        return fn
    return decorator
