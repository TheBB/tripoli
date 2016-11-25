import pytest

from string import ascii_lowercase
from tripoli.types import List
from tripoli import _
from emacs import list as mklist, set as setq
import emacs_raw as er


def em_list(chars):
    return mklist(*(_(c) for c in chars))

def py_list(chars):
    return [_(c) for c in chars]


def test_construct():
    eo = em_list('abc')

    l = List(eo)
    assert list(l) == py_list('abc')
    assert not l.bindable

    setq(_('test'), eo)
    l = List(_('test'))
    assert list(l) == py_list('abc')
    assert l.bindable

    import emacs
    l = List(emacs.test)
    assert list(l) == py_list('abc')
    assert l.bindable


def test_getitem():
    eo = em_list('abcde')

    l = List(eo)
    assert l[0] == _('a')
    assert l[1] == _('b')
    assert l[2] == _('c')
    assert l[3] == _('d')
    assert l[4] == _('e')


def test_setitem():
    eo = em_list('abc')

    l = List(eo)
    l[1] = _('y')
    assert list(l) == py_list('ayc')
    l[0] = _('q')
    assert list(l) == py_list('qyc')


def test_delitem():
    eo = em_list('abcde')

    l = List(eo)
    del l[1]
    assert list(l) == py_list('acde')

    with pytest.raises(TypeError):
        del l[0]

    eo = em_list('abcde')
    setq(_('test'), eo)
    l = List(_('test'))
    del l[0]
    assert list(l) == py_list('bcde')


def test_delete():
    eo = em_list(ascii_lowercase)

    l = List(eo)
    with pytest.raises(TypeError):
        l.delete([0])

    setq(_('test'), eo)
    l = List(_('test'))
    l.delete([0, 4, 9, 10, 11, 25])
    assert list(l) == py_list('bcdfghimnopqrstuvwxy')


def test_empty():
    l = List()

    with pytest.raises(TypeError):
        l.push(_('a'))

    setq(_('test'), er.intern('nil'))
    l = List(_('test'))
    l.push(_('a'))
    l.push(_('b'))
    l.push(_('c'))
    assert list(l) == py_list('cba')


def test_insert():
    l = List()

    with pytest.raises(TypeError):
        l.insert(0, _('a'))

    setq(_('test'), er.intern('nil'))
    l = List(_('test'))
    l.insert(0, _('a'))
    l.insert(0, _('b'))
    l.insert(1, _('c'))
    assert list(l) == py_list('bca')

    l.insert(3, _('d'))
    assert list(l) == py_list('bcad')


def test_clear():
    eo = em_list(ascii_lowercase)
    l = List(eo)

    with pytest.raises(TypeError):
        l.clear()

    setq(_('test'), eo)
    l = List(_('test'))
    l.clear()
    assert list(l) == []
