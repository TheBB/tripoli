import emacs_raw
from enum import Enum
from importlib import import_module
from importlib.machinery import ModuleSpec
from itertools import groupby, product


class AmbiguousSymbolError(NameError):
    pass

class UnboundSymbolError(NameError):
    pass


class EmacsNamespaceFinder:

    def find_spec(self, name, path, target):
        if not name.startswith('emacs'):
            return None
        module = EmacsNamespace()
        for p in name.split('.')[1:]:
            module = getattr(module, p)
        return ModuleSpec(name, EmacsNamespaceLoader(module), origin='emacs')


class EmacsNamespaceLoader:

    def __init__(self, module):
        self.module = module

    def create_module(self, spec):
        return self.module

    def exec_module(self, module):
        pass


class EmacsNamespace:

    def __init__(self, prefix=None, seps='-/:|'):
        self.__prefix = [] if prefix is None else prefix
        self.__separators = seps
        self.__clear_cache()

    def __sub(self, **kwargs):
        new_kwargs = {'prefix': self.__prefix, 'seps': self.__separators}
        new_kwargs.update(kwargs)
        return EmacsNamespace(**new_kwargs)

    def __getitem__(self, index):
        if isinstance(index, str):
            return self.__sub(prefix=self.__prefix + [[index]])

    def __clear_cache(self):
        self.__cached_subs = {}

    def __getattr_special(self, name):
        if name == 'raw_' and not self.__prefix:
            return import_module('emacs_raw')
        elif name == 'clear_cache_':
            return self.__clear_cache
        elif name == 'fs_':
            return self.__function_symbol
        elif name == 'vs_':
            return self.__variable_symbol
        elif name == 'fb_':
            sym = self.__function_symbol()
            return emacs_raw.intern('symbol-function')(sym)
        elif name == 'vb_':
            sym = self.__variable_symbol()
            return emacs_raw.intern('symbol-value')(sym)
        elif name == 'prefix_':
            return self.__prefix
        elif name == 'seps_':
            return lambda s: self.__sub(seps=s)
        elif name == 'symbols_':
            return self.__symbols

    def __getattr__(self, name):
        print('__getattr__', name)
        if name.endswith('_'):
            return self.__getattr_special(name)
        if name in self.__cached_subs:
            return self.__cached_subs[name]

        tname = name
        seps = self.__separators
        if self.__prefix:
            finals = {c[-1] for c in self.__prefix[-1]}
            if any(c.isalnum() for c in finals):
                tname = '_' + tname
            if len(finals) == 1:
                c = next(iter(finals))
                if not c.isalnum():
                    seps = c[-1]

        new_prefix = []
        for is_sep, chars in groupby(tname, lambda c: c == '_'):
            if is_sep:
                ilen = sum(1 for _ in chars)
                new_prefix.append([c*ilen for c in seps])
            else:
                new_prefix.append([''.join(chars)])
            seps = self.__separators

        ret = self.__sub(prefix=self.__prefix + new_prefix)
        self.__cached_subs[name] = ret
        return ret

    def __symbols(self, convert=True):
        iters = []
        for parts in product(*self.__prefix[::-1]):
            name = ''.join(parts[::-1])
            if convert: yield emacs_raw.intern(name)
            else: yield name

    def __symbol_satisfying(self, predicate, exists):
        found = None
        for s in self.__symbols():
            print('Testing', s)
            if predicate(s):
                if found:
                    raise AmbiguousSymbolError()
                print('Setting to', s)
                found = s
        if found is None and exists:
            print('Unbound, raising')
            raise UnboundSymbolError()
        elif found is None:
            sym = next(self.__symbols())
            print('Unbound, returning', sym)
            return sym
        return found

    def __function_symbol(self, exists=True):
        return self.__symbol_satisfying(emacs_raw.intern('fboundp'), exists=exists)

    def __variable_symbol(self, exists=True):
        return self.__symbol_satisfying(emacs_raw.intern('boundp'), exists=exists)

    def __call__(self, *args, **kwargs):
        if kwargs:
            args = list(args)
            for k, v in kwargs.items():
                args.append(emacs_raw.intern(':' + k))
                args.append(v)
        func = self.__function_symbol()
        return func(*args)

    def __setattr__(self, name, value):
        if name.startswith('_EmacsNamespace__') or name.startswith('__') or name.endswith('_'):
            self.__dict__[name] = value
            return
        if isinstance(value, EmacsNamespace):
            return
        sym = getattr(self, name).vs_(exists=False)
        emacs_raw.intern('set')(sym, value)

    def __setitem__(self, name, value):
        sym = self[name].vs_(exists=False)
        emacs_raw.intern('set')(sym, value)
