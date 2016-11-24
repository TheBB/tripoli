import pytest

from tripoli.types import AssociationList, List
from tripoli import _
from emacs import list as mklist, set as setq, cons
import emacs_raw as er


def test_getitem():
    eo = mklist(
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=False)
    assert alist[_('a')] == 1
    assert alist[_('b')] == 2
    assert alist[_('c')] == 3
    with pytest.raises(KeyError):
        alist[_('d')]
    with pytest.raises(KeyError):
        alist['a']

    eo = mklist(
        cons(er.str('a'), er.int(1)),
        cons(er.str('b'), er.int(2)),
        cons(er.str('c'), er.int(3)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=False)
    with pytest.raises(KeyError):
        alist[er.str('a')]
    alist = AssociationList(eo, assq=False, prefer_symbol=False)
    assert alist[er.str('a')] == 1
    assert alist[er.str('b')] == 2
    assert alist[er.str('c')] == 3
    assert alist['a'] == 1
    assert alist['b'] == 2
    assert alist['c'] == 3
    with pytest.raises(KeyError):
        alist[er.str('d')]

    eo = mklist(
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=True)
    assert alist[_('a')] == 1
    assert alist[_('b')] == 2
    assert alist[_('c')] == 3
    assert alist['a'] == 1
    assert alist['b'] == 2
    assert alist['c'] == 3


def test_setitem():
    eo = mklist(
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=True, inplace=True)
    alist['a'] = 5
    alist['d'] = 6

    assert alist['a'] == 5
    assert alist['b'] == 2
    assert alist['c'] == 3
    assert alist['d'] == 6

    l = List(alist.place)
    assert len(l) == 4
    assert len(alist) == 4

    eo = mklist(
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=True, inplace=False)
    alist['a'] = 5
    alist['d'] = 6

    assert alist['a'] == 5
    assert alist['b'] == 2
    assert alist['c'] == 3
    assert alist['d'] == 6

    l = List(alist.place)
    assert len(l) == 5
    assert len(alist) == 4


def test_delitem_len():
    eo = mklist(
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=True)

    with pytest.raises(TypeError):
        del alist['a']
    assert alist['b'] == 2
    del alist['b']
    assert len(alist) == 2
    with pytest.raises(KeyError):
        alist['b']

    eo = mklist(
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
    )
    setq(_('test'), eo)
    alist = AssociationList(_('test'), assq=True, prefer_symbol=True)

    assert alist['a'] == 1
    del alist['a']
    with pytest.raises(KeyError):
        alist['a']
    assert alist['b'] == 2
    del alist['b']
    assert len(alist) == 1
    with pytest.raises(KeyError):
        alist['b']


def test_iter():
    eo = mklist(
        cons(_('d'), er.int(0)),
        cons(_('a'), er.int(1)),
        cons(_('b'), er.int(2)),
        cons(_('c'), er.int(3)),
        cons(_('d'), er.int(0)),
    )
    alist = AssociationList(eo, assq=True, prefer_symbol=True)
    assert list(alist) == [_('d'), _('a'), _('b'), _('c')]
    assert list(alist.keys()) == [_('d'), _('a'), _('b'), _('c')]
    assert list(alist.values()) == [0, 1, 2, 3]
    assert list(alist.items()) == [
        (_('d'), 0),
        (_('a'), 1),
        (_('b'), 2),
        (_('c'), 3),
    ]
