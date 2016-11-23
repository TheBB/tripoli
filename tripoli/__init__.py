import sys
from .namespace import (
    EmacsNamespaceFinder, EmacsNamespace, UnboundSymbolError, AmbiguousSymbolError
)

sys.meta_path.append(EmacsNamespaceFinder())
sys.modules['emacs'] = EmacsNamespace()

from .util import emacsify_args, symbolify_args

import emacs as e
import emacs_raw as er
require = symbolify_args()(e.require)
_ = er.intern
