from collections import OrderedDict
import pytest

from tripoli.types import PList
from emacs_raw import intern as _


def test_setitem():
    plist = PList()
    plist['a'] = 'b'
    plist['b'] = 'c'
    plist['c'] = 'd'
    assert list(plist.place) == [_('c'), 'd', _('b'), 'c', _('a'), 'b']

    plist = PList(colonify=True)
    plist['a'] = 'b'
    plist['b'] = 'c'
    plist[':c'] = 'd'
    assert list(plist.place) == [_(':c'), 'd', _(':b'), 'c', _(':a'), 'b']
    plist[_(':b')] = 'c'
    assert list(plist.place) == [_(':b'), 'c', _(':c'), 'd', _(':b'), 'c', _(':a'), 'b']

    plist = PList(consistent=True)
    plist[_('a')] = 'b'
    plist['b'] = 'c'
    plist['c'] = 'd'
    assert list(plist.place) == [_('c'), 'd', _('b'), 'c', _('a'), 'b']
    plist['b'] = 3
    assert list(plist.place) == [_('c'), 'd', _('b'), 3, _('a'), 'b']


def test_initializer():
    plist = PList(OrderedDict(a=1, b=2))
    assert list(plist.place) == [_('a'), 1, _('b'), 2]

    plist = PList(OrderedDict(a=1, b='c'), prefer_symbol=True)
    assert list(plist.place) == [_('a'), 1, _('b'), _('c')]

    plist = PList([('a', 1), ('b', 2)])
    assert list(plist.place) == [_('a'), 1, _('b'), 2]

    plist = PList((p for p in [('a', 1), ('b', 2)]))
    assert list(plist.place) == [_('a'), 1, _('b'), 2]


def test_iterators():
    plist = PList(OrderedDict(a=1, b='c', c='d', d=81))
    assert list(plist.keys()) == [_('a'), _('b'), _('c'), _('d')]
    assert list(plist.values()) == [1, 'c', 'd', 81]
    assert list(plist.items()) == [(_('a'), 1), (_('b'), 'c'), (_('c'), 'd'), (_('d'), 81)]


def test_getitem():
    plist = PList(OrderedDict(a=1, b='c', c='d', d=81))

    assert plist['a'] == 1
    assert plist['b'] == 'c'
    assert plist['c'] == 'd'
    assert plist[_('d')] == 81

    with pytest.raises(KeyError):
        plist['e']

    plist = PList(OrderedDict(a=1, b='c', c='d', d=81), colonify=True)

    assert plist[':a'] == 1
    assert plist['b'] == 'c'
    assert plist[':c'] == 'd'
    assert plist[_('d')] == 81

    with pytest.raises(KeyError):
        plist['e']


def test_delitem():
    plist = PList(OrderedDict(a=1, b='c', c='d', d=81))

    del plist['a']
    assert list(plist.items()) == [(_('b'), 'c'), (_('c'), 'd'), (_('d'), 81)]
    del plist['d']
    assert list(plist.items()) == [(_('b'), 'c'), (_('c'), 'd')]

    with pytest.raises(KeyError):
        del plist['e']

    plist = PList(colonify=True)
    plist['a'] = 1
    plist['a'] = 2
    plist['b'] = 3
    plist['a'] = 4
    assert list(plist.place) == [_(':a'), 4, _(':b'), 3, _(':a'), 2, _(':a'), 1]

    del plist['a']
    assert list(plist.place) == [_(':b'), 3]

    plist = PList(colonify=True, consistent=True)
    plist['a'] = 1
    plist['a'] = 2
    plist['b'] = 3
    plist['a'] = 4
    assert list(plist.place) == [_(':b'), 3, _(':a'), 4]

    del plist['a']
    assert list(plist.place) == [_(':b'), 3]

    # Deleting from a list that is assumed to be consistent avoids checking if
    # it really is
    l = _('list')(_('a'), 1, _('a'), 2)
    plist = PList(bind=l, consistent=True)
    assert list(plist.place) == [_('a'), 1, _('a'), 2]
    del plist['a']
    assert list(plist.place) == [_('a'), 2]
