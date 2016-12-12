from . import _
from .types import List
from .util import coerce
import emacs_raw as er


def _function(fn, name=None, interactive=None):
    efn = er.function(fn)

    if interactive:
        if interactive is True:
            interactive = er.intern('list')(_('interactive'))
        elif interactive:
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
    """defun(name=True, interactive=None)

    Decorator for defining an Emacs function and binding it to a symbol.

    :param name: The symbol to which to bind the function. If `true`, uses the
        Python name of the function with underscores replaced by hyphens.
    :type name: symbol-like or `True`
    :param interactive: Optional interactive spec.
    :type interactive: `bool` or coercible to :class:`.EmacsObject`
    """
    def decorator(fn):
        _function(fn, name=name, **kwargs)
        return fn
    return decorator


@coerce(only={'hook'}, prefer_symbol={'hook'})
def add_hook(hook, append=False, local=False, **kwargs):
    """add_hook(append=False, local=False, name=None, interactive=None)

    Decorator for defining an Emacs function and adding it to a hook.

    :param symbol-like hook: The hook to which to add the function.
    :param bool append: If true, adds to the end of the hook instead of the
        beginning.
    :param bool local: If true, adds to the buffer-local value of the hook.
    :param name: An optional name to which to bind the function. If `true`,
        uses the Python name of the function with underscores replaced by
        hyphens.
    :type name: symbol-like or `True`
    :param interactive: Optional interactive spec.
    :type interactive: `bool` or coercible to :class:`.EmacsObject`
    """
    hook = er.EmacsObject(hook, require_symbol=True)

    def decorator(fn):
        efn = _function(fn, **kwargs)
        er.intern('add-hook')(hook, efn, append, local)
        return fn
    return decorator


@coerce(only={'feature'}, prefer_symbol={'feature'})
def eval_after_load(feature, **kwargs):
    """eval_after_load(feature, name=None, interactive=None)

    Decorator for defining an Emacs function and running it after a feature has
    loaded (or now, if already loaded).

    :param symbol-like feature: The feature after which to run the function.
    :param name: An optional name to which to bind the function. If `true`,
        uses the Python name of the function with underscores replaced by
        hyphens.
    :type name: symbol-like or `True`
    :param interactive: Optional interactive spec.
    :type interactive: `bool` or coercible to :class:`.EmacsObject`

    """
    feature = er.EmacsObject(feature)

    def decorator(fn):
        er.intern('eval-after-load')(feature, _function(fn, **kwargs))
        return fn
    return decorator
