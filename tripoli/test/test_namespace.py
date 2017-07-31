import pytest

import emacs_raw as e
from tripoli.namespace import EmacsNamespace, syms, seps, sym, fbinding, binding, fbound, bound


root = EmacsNamespace()
a = {'convert': False}


def test_basic():
    assert set(root.package[syms(**a)]) == {'package'}
    assert set(root.package.initialize[syms(**a)]) == {
        'package-initialize', 'package/initialize',
        'package:initialize', 'package|initialize',
    }
    assert set(root.package._initialize[syms(**a)]) == {
        'package--initialize', 'package//initialize',
        'package::initialize', 'package||initialize',
    }
    assert set(root.with_underscore[syms(**a)]) == {
        'with-underscore', 'with/underscore',
        'with:underscore', 'with|underscore',
    }
    assert set(root.a.b.c[syms(**a)]) == {
        'a-b-c', 'a-b/c', 'a-b:c', 'a-b|c',
        'a/b-c', 'a/b/c', 'a/b:c', 'a/b|c',
        'a:b-c', 'a:b/c', 'a:b:c', 'a:b|c',
        'a|b-c', 'a|b/c', 'a|b:c', 'a|b|c',
    }


def test_index():
    spacemacs = root.spacemacs
    assert set(spacemacs['--'][syms(**a)]) == {'spacemacs--'}
    assert set(spacemacs['--'].stuff[syms(**a)]) == {'spacemacs--stuff'}
    assert set(spacemacs['--']._stuff[syms(**a)]) == {'spacemacs---stuff'}
    assert set(spacemacs._stuff[syms(**a)]) == {
        'spacemacs--stuff', 'spacemacs//stuff',
        'spacemacs::stuff', 'spacemacs||stuff',
    }
    assert set(spacemacs['/']._stuff[syms(**a)]) == {'spacemacs//stuff'}
    assert set(spacemacs['/']._with_underscore[syms(**a)]) == {
        'spacemacs//with-underscore', 'spacemacs//with/underscore',
        'spacemacs//with:underscore', 'spacemacs//with|underscore',
    }


def test_custom_seps():
    loff = root.loff[seps('~^')]
    assert set(loff.ding[syms(**a)]) == {'loff~ding', 'loff^ding'}
    assert set(loff._ding[syms(**a)]) == {'loff~~ding', 'loff^^ding'}

    doff = loff.ding[seps('$')].doff
    assert set(doff[syms(**a)]) == {'loff~ding$doff', 'loff^ding$doff'}


def test_import():
    import emacs
    assert isinstance(emacs, EmacsNamespace)
    assert set(emacs[syms(**a)]) == {''}

    from emacs import lel
    assert set(lel[syms(**a)]) == {'lel'}

    import emacs.rofl as rofl
    assert set(rofl[syms(**a)]) == {'rofl'}

    import emacs.a.b as ab
    assert set(ab[syms(**a)]) == {'a-b', 'a/b', 'a:b', 'a|b'}


def test_sym():
    from emacs.emacs import version
    assert version[sym] == e.intern('emacs-version')

    from emacs.doesnt import exist
    assert exist[sym] == e.intern('doesnt-exist')

    import emacs
    beta = emacs[seps('$')].alpha.beta
    assert beta[sym] == e.intern('alpha$beta')


def test_fbinding():
    get = lambda s: e.intern('symbol-function')(e.intern(s))

    from emacs.emacs import version
    assert e.eq(version[fbinding], get('emacs-version'))

    import emacs
    with pytest.raises(NameError):
        emacs.doesnt.exist[fbinding]


def test_binding():
    get = lambda s: e.intern('symbol-value')(e.intern(s))

    from emacs.emacs import version
    assert e.eq(version[binding], get('emacs-version'))

    import emacs
    with pytest.raises(NameError):
        emacs.doesnt.exist[fbinding]


def test_fbound():
    from emacs.emacs import version
    assert version[fbound()] == e.intern('emacs-version')

    import emacs
    with pytest.raises(NameError):
        emacs.doesnt.exist[fbound()]

    assert emacs.doesnt.exist[fbound(exists=False)] == e.intern('doesnt-exist')


def test_bound():
    from emacs.emacs import version
    assert version[bound()] == e.intern('emacs-version')

    import emacs
    with pytest.raises(NameError):
        emacs.doesnt.exist[bound()]

    assert emacs.doesnt.exist[bound(exists=False)] == e.intern('doesnt-exist')


def test_setattr():
    from emacs import test

    test.symbol = 'alpha'
    assert e.intern('symbol-value')(e.intern('test-symbol')) == 'alpha'


def test_call():
    def a():
        return e.int(1)
    func = e.function(a, 0, 0)
    e.intern('fset')(e.intern('test-symbol'), func)

    from emacs import test
    assert test.symbol() == e.int(1)
