from . import _
from .types import List
import emacs_raw as er


def function(fn, name=None, interactive=None):
    efn = er.function(fn)

    if interactive:
        if interactive is True:
            interactive = er.intern('list')(_('interactive'))
        else:
            interactive = er.intern('list')(_('interactive'), interactive)

        form = List(efn)
        assert form[0] == _('lambda')

        index = 2
        if not form[index] or er.stringp(form[index]):
            index += 1

        car = er.intern('car')
        if er.consp(form[index]) and car(form[index]) == _('interactive'):
            form[index] = interactive
        else:
            form.insert(index, interactive)

    if name:
        if not isinstance(name, str):
            name = fn.__name__.replace('_', '-')
        name = er.EmacsObject(name, require_symbol=True)
        er.intern('fset')(name, efn)
        return name
    return efn


def defun(name=True, **kwargs):
    def decorator(fn):
        function(fn, name=name, **kwargs)
        return fn
    return decorator


def add_hook(hook, append=False, local=False, **kwargs):
    hook = er.EmacsObject(hook, require_symbol=True)

    def decorator(fn):
        efn = function(fn, **kwargs)
        er.intern('add-hook')(hook, efn, append, local)
        return fn
    return decorator


def eval_after_load(feature, **kwargs):
    feature = er.EmacsObject(feature)

    def decorator(fn):
        er.intern('eval-after-load')(feature, function(fn, **kwargs))
        return fn
    return decorator
