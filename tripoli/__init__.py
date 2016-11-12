import sys
from .namespace import (
    EmacsNamespaceFinder, EmacsNamespace, UnboundSymbolError, AmbiguousSymbolError
)

sys.meta_path.append(EmacsNamespaceFinder())
sys.modules['emacs'] = EmacsNamespace()
