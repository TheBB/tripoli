import pytest
from string import ascii_lowercase

from tripoli.types import List
import emacs_raw as er


_ = er.intern
mklist = _('list')
setq = _('set')


def em_list(chars):
    return mklist(*(_(c) for c in chars))

def py_list(chars):
    return [_(c) for c in chars]


def test_construct():
    eo = em_list('abc')

    l = List(bind=eo)
    assert list(l) == py_list('abc')

    setq(_('test'), eo)
    l = List(bind='test')
    assert list(l) == py_list('abc')

    import emacs
    l = List(bind=emacs.test)
    assert list(l) == py_list('abc')


def test_getitem():
    eo = em_list('abcde')

    l = List(bind=eo)
    assert l[0] == _('a')
    assert l[1] == _('b')
    assert l[2] == _('c')
    assert l[3] == _('d')
    assert l[4] == _('e')

    with pytest.raises(IndexError):
        l[5]


def test_setitem():
    eo = em_list('abc')

    l = List(bind=eo)
    l[1] = _('y')
    assert list(l) == py_list('ayc')
    l[0] = _('q')
    assert list(l) == py_list('qyc')


def test_delitem():
    eo = em_list('abcde')

    l = List(bind=eo)
    del l[1]
    assert list(l) == py_list('acde')

    eo = em_list('abcde')
    setq(_('test'), eo)
    l = List(bind='test')
    del l[0]
    assert list(l) == py_list('bcde')

    with pytest.raises(IndexError):
        del l[10]


def test_delete():
    eo = em_list(ascii_lowercase)

    l = List(bind=eo)

    setq(_('test'), eo)
    l = List(bind='test')
    l.delete([0, 4, 9, 10, 11, 25])
    assert list(l) == py_list('bcdfghimnopqrstuvwxy')


def test_empty():
    l = List()

    setq(_('test'), er.intern('nil'))
    l = List(bind='test')
    l.push(_('a'))
    l.push(_('b'))
    l.push(_('c'))
    assert list(l) == py_list('cba')


def test_insert():
    l = List()

    setq(_('test'), er.intern('nil'))
    l = List(bind=_('test'))
    l.insert(0, _('a'))
    l.insert(0, _('b'))
    l.insert(1, _('c'))
    assert list(l) == py_list('bca')

    l.insert(3, _('d'))
    assert list(l) == py_list('bcad')


def test_clear():
    eo = em_list(ascii_lowercase)
    l = List(bind=eo)

    setq(_('test'), eo)
    l = List(bind=_('test'))
    l.clear()
    assert list(l) == []


def test_initializer():
    l = List(ascii_lowercase)
    assert list(l) == list(ascii_lowercase)

    l = List(ascii_lowercase, prefer_symbol=True)
    assert list(l) == py_list(ascii_lowercase)

    l = List(ascii_lowercase, bind='test')
    assert er.eq(l.place, _('symbol-value')(_('test')))
    assert list(l) == list(ascii_lowercase)

    l = List((a for a in ascii_lowercase))
    assert list(l) == list(ascii_lowercase)
