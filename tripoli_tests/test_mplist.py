from collections import OrderedDict
import pytest

from tripoli.types import MPList
import emacs_raw as er


_ = er.intern
elist = _('list')


def test_setitem():
    mplist = MPList()
    mplist['a'] = [1, 2, 3]
    mplist['b'] = [3, 4]
    mplist['c'] = []
    assert list(mplist.place) == [_(':c'), _(':b'), 3, 4, _(':a'), 1, 2, 3]
    mplist['c'] = [8]
    assert list(mplist.place) == [_(':c'), 8, _(':c'), _(':b'), 3, 4, _(':a'), 1, 2, 3]

    mplist = MPList(consistent=True)
    mplist['a'] = []
    mplist['b'] = [1, 2]
    assert list(mplist.place) == [_(':b'), 1, 2, _(':a')]
    mplist['a'] = [3]
    assert list(mplist.place) == [_(':b'), 1, 2, _(':a'), 3]
    mplist['b'] = []
    assert list(mplist.place) == [_(':b'), _(':a'), 3]
    mplist['a'] = []
    assert list(mplist.place) == [_(':b'), _(':a')]


def test_initializer():
    mplist = MPList(OrderedDict(a=[1], b=[2]))
    assert list(mplist.place) == [_(':a'), 1, _(':b'), 2]

    mplist = MPList([('a', [2, 3]), ('b', []), ('c', [1])])
    assert list(mplist.place) == [_(':a'), 2, 3, _(':b'), _(':c'), 1]

    mplist = MPList((p for p in [('a', [1]), ('b', [2])]))
    assert list(mplist.place) == [_(':a'), 1, _(':b'), 2]


def test_iterators():
    mplist = MPList(OrderedDict(a=[1], b=['c'], c=['d'], d=[]))
    assert list(mplist.keys()) == [_(':a'), _(':b'), _(':c'), _(':d')]
    assert list(mplist.values()) == [elist(1), elist('c'), elist('d'), elist()]
    assert list(mplist.items()) == [
        (_(':a'), elist(1)), (_(':b'), elist('c')), (_(':c'), elist('d')), (_(':d'), elist())
    ]


def test_getitem():
    mplist = MPList(OrderedDict(a=[1,2], b=[3], c=[], d=[]))

    assert mplist['a'] == elist(1, 2)
    assert mplist['b'] == elist(3)
    assert mplist['c'] == elist()
    assert mplist['d'] == elist()

    with pytest.raises(KeyError):
        mplist['e']

    mplist = MPList(OrderedDict(a=[], b=['a', 1, 2]))

    assert mplist['a'] == elist()
    assert mplist['b'] == elist('a', 1, 2)

    with pytest.raises(KeyError):
        mplist['c']


def test_delitem():
    mplist = MPList(OrderedDict(a=[1,2,3], b=[4,5], c=[]))

    del mplist['a']
    assert list(mplist.place) == [_(':b'), 4, 5, _(':c')]
    del mplist['c']
    assert list(mplist.place) == [_(':b'), 4, 5]

    with pytest.raises(KeyError):
        del mplist['e']

    l = _('list')(_(':a'), 1, _(':a'), 2, 3, _(':b'), 7)
    mplist = MPList(bind=l)
    del mplist['a']
    assert list(mplist.place) == [_(':b'), 7]

    # Deleting from a list that is assumed to be consistent avoids checking if
    # it really is
    l = _('list')(_(':a'), 1, _(':a'), 2, 3, _(':b'), 7)
    mplist = MPList(bind=l, consistent=True)
    del mplist['a']
    assert list(mplist.place) == [_(':a'), 2, 3, _(':b'), 7]
