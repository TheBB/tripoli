from tripoli.util import PlaceOrSymbol

import emacs_raw as er


def test_place():
    obj = PlaceOrSymbol()

    assert er.eq(obj.place, er.intern('nil'))
    assert er.eq(er.EmacsObject(obj), er.intern('nil'))

    new_place = er.list([er.int(1), er.int(2), er.int(3)])
    obj._bind(new_place)

    assert er.eq(obj.place, new_place)
    assert er.eq(er.EmacsObject(obj), new_place)


def test_symbol():
    mylist = er.list([er.int(1), er.int(2), er.int(3)])

    sym = er.intern('alpha')
    setq = er.intern('set')
    setq(sym, mylist)

    obj = PlaceOrSymbol(sym)
    assert er.eq(obj.place, mylist)
    assert er.eq(er.EmacsObject(obj), mylist)

    mylist = er.list([er.int(1), er.int(2), er.int(3), er.int(4)])
    obj._bind(mylist)

    assert er.eq(obj._symbol, er.intern('alpha'))
    assert er.eq(obj.place, mylist)
    assert er.eq(er.EmacsObject(obj), mylist)
    assert er.eq(er.intern('symbol-value')(er.intern('alpha')), mylist)
