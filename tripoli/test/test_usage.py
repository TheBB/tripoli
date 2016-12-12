import pytest

import emacs_raw as e
import emacs


def test_funcall():
    sym = emacs.intern(e.str('hello'))
    assert isinstance(sym, e.EmacsObject)
    assert e.symbolp(sym)
    assert str(sym) == 'hello'

    with pytest.raises(NameError):
        emacs.no_such_function()


def test_assign():
    value = e.intern('symbol-value')

    emacs.var_name = e.intern('omfg')
    val = value(e.intern('var-name'))
    assert e.symbolp(val)
    assert str(val) == 'omfg'

    emacs['var-name-'] = e.intern('lol')
    val = value(e.intern('var-name-'))
    assert e.symbolp(val)
    assert str(val) == 'lol'
