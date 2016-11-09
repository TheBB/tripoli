import emacs
import pytest
from tripoli.namespace import Namespace

root = Namespace()
a = {'convert': False}

def test_basic():
    assert set(root.package.symbols_(**a)) == {'package'}
    assert set(root.package.initialize.symbols_(**a)) == {
        'package-initialize', 'package/initialize',
        'package:initialize', 'package|initialize',
    }
    assert set(root.package._initialize.symbols_(**a)) == {
        'package--initialize', 'package//initialize',
        'package::initialize', 'package||initialize',
    }
    assert set(root.with_underscore.symbols_(**a)) == {
        'with-underscore', 'with/underscore',
        'with:underscore', 'with|underscore',
    }
    assert set(root.a.b.c.symbols_(**a)) == {
        'a-b-c', 'a-b/c', 'a-b:c', 'a-b|c',
        'a/b-c', 'a/b/c', 'a/b:c', 'a/b|c',
        'a:b-c', 'a:b/c', 'a:b:c', 'a:b|c',
        'a|b-c', 'a|b/c', 'a|b:c', 'a|b|c',
    }

def test_index():
    spacemacs = root.spacemacs
    assert set(spacemacs['--'].symbols_(**a)) == {'spacemacs--'}
    assert set(spacemacs['--'].stuff.symbols_(**a)) == {'spacemacs--stuff'}
    assert set(spacemacs['--']._stuff.symbols_(**a)) == {'spacemacs---stuff'}
    assert set(spacemacs._stuff.symbols_(**a)) == {
        'spacemacs--stuff', 'spacemacs//stuff',
        'spacemacs::stuff', 'spacemacs||stuff',
    }
    assert set(spacemacs['/']._stuff.symbols_(**a)) == {'spacemacs//stuff'}
    assert set(spacemacs['/']._with_underscore.symbols_(**a)) == {
        'spacemacs//with-underscore', 'spacemacs//with/underscore',
        'spacemacs//with:underscore', 'spacemacs//with|underscore',
    }

def test_custom_seps():
    loff = root.loff.seps_('~^')
    assert set(loff.ding.symbols_(**a)) == {'loff~ding', 'loff^ding'}
    assert set(loff._ding.symbols_(**a)) == {'loff~~ding', 'loff^^ding'}

    doff = loff.ding.seps_('$').doff
    assert set(doff.symbols_(**a)) == {'loff~ding$doff', 'loff^ding$doff'}

def test_v_f():
    # Dinstinguishing between functions and variables,
    # package-initialize is a function, and package--initialized is a variable
    require = emacs.intern('require')
    require(emacs.intern('package'))

    assert root.package.initialize.s_.is_callable()
    root.package._initialized.s_
    with pytest.raises(NameError):
        root.package.v_.initialize.s_
    with pytest.raises(NameError):
        root.f_.package._initialized.s_
