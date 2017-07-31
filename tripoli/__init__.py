import sys

from .namespace import EmacsNamespaceFinder, EmacsNamespace

sys.meta_path.append(EmacsNamespaceFinder())
sys.modules['emacs'] = EmacsNamespace()
