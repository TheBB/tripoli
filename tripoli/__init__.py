from functools import wraps
import sys
from .namespace import EmacsNamespaceFinder, EmacsNamespace

sys.meta_path.append(EmacsNamespaceFinder())
sys.modules['emacs'] = EmacsNamespace()

from .util import coerce, symbolify

__all__ = ['require', '_']

import emacs as e
import emacs_raw as er

@symbolify()
def require(name):
    """Requires the given feature. Equivalent to :lisp:`(require name)` in
    elisp."""
    return e.require(name)

def _(name):
    """Creates an :class:`.EmacsObject` corresponding to the interned symbol with
    the given name. Equivalent to :lisp:`(intern name)` in elisp.
    """
    return er.intern(name)
