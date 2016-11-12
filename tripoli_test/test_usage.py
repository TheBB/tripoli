import tripoli

import pytest
import emacs.raw_ as e
import emacs

from tripoli import UnboundSymbolError

def test_funcall():
    sym = emacs.intern(e.str('hello'))
    assert isinstance(sym, e.EmacsObject)
    assert sym.is_symbol()
    assert str(sym) == 'hello'

    with pytest.raises(UnboundSymbolError):
        emacs.no_such_function()

def test_assign():
    value = e.intern('symbol-value')

    emacs.var_name = e.intern('omfg')
    val = value(e.intern('var-name'))
    assert val.is_symbol()
    assert str(val) == 'omfg'

    emacs['var-name-'] = e.intern('lol')
    val = value(e.intern('var-name-'))
    assert val.is_symbol()
    assert str(val) == 'lol'
