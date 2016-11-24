import pytest

import emacs_raw as e


def test_nil():
    nil = e.intern('nil')
    assert e.eq(nil, e.intern('nil'))
    assert repr(nil) == 'nil'
    assert str(nil) == 'nil'
    with pytest.raises(TypeError):
        int(nil)
    with pytest.raises(TypeError):
        float(nil)
    assert nil.type() == 'symbol'
    assert nil.is_a('symbol')
    assert not nil.is_int()
    assert not nil.is_float()
    assert not nil.is_str()
    assert nil.is_symbol()
    assert not nil.is_cons()
    assert not nil.is_vector()
    assert nil.is_list()
    assert not nil.is_callable()
    assert not nil


def test_intern():
    alpha = e.intern('alpha')
    assert e.eq(alpha, e.intern('alpha'))
    assert repr(alpha) == 'alpha'
    assert str(alpha) == 'alpha'
    with pytest.raises(TypeError):
        int(alpha)
    with pytest.raises(TypeError):
        float(alpha)
    assert alpha.type() == 'symbol'
    assert alpha.is_a('symbol')
    assert not alpha.is_int()
    assert not alpha.is_float()
    assert not alpha.is_str()
    assert alpha.is_symbol()
    assert not alpha.is_cons()
    assert not alpha.is_vector()
    assert not alpha.is_list()
    assert not alpha.is_callable()
    assert alpha

    with pytest.raises(TypeError):
        e.intern(2)
    with pytest.raises(TypeError):
        e.intern([])


def test_int():
    one = e.int(1)
    assert e.eq(one, e.int(1))
    assert repr(one) == '1'
    assert str(one) == '1'
    assert int(one) == 1
    assert float(one) == 1.0
    assert one.type() == 'integer'
    assert one.is_a('integer')
    assert one.is_int()
    assert not one.is_float()
    assert not one.is_str()
    assert not one.is_symbol()
    assert not one.is_cons()
    assert not one.is_vector()
    assert not one.is_list()
    assert not one.is_callable()
    assert one

    zero = e.int(0)
    assert e.eq(zero, e.int(0))
    assert repr(zero) == '0'
    assert str(zero) == '0'
    assert int(zero) == 0
    assert float(zero) == 0.0
    assert zero.type() == 'integer'
    assert zero.is_a('integer')
    assert zero.is_int()
    assert not zero.is_float()
    assert not zero.is_str()
    assert not zero.is_symbol()
    assert not zero.is_cons()
    assert not zero.is_vector()
    assert not zero.is_list()
    assert not zero.is_callable()
    assert zero

    with pytest.raises(TypeError):
        e.int(0.1)
    with pytest.raises(TypeError):
        e.int('alpha')


def test_float():
    one = e.float(1.1)
    assert e.eql(one, e.float(1.1))
    assert repr(one) == '1.1'
    assert str(one) == '1.1'
    assert int(one) == 1
    assert float(one) == 1.1
    assert one.type() == 'float'
    assert one.is_a('float')
    assert not one.is_int()
    assert one.is_float()
    assert not one.is_str()
    assert not one.is_symbol()
    assert not one.is_cons()
    assert not one.is_vector()
    assert not one.is_list()
    assert not one.is_callable()
    assert one

    zero = e.float(0)
    assert e.eql(zero, e.float(0))
    assert repr(zero) == '0.0'
    assert str(zero) == '0.0'
    assert int(zero) == 0
    assert float(zero) == 0.0
    assert zero.type() == 'float'
    assert zero.is_a('float')
    assert not zero.is_int()
    assert zero.is_float()
    assert not zero.is_str()
    assert not zero.is_symbol()
    assert not zero.is_cons()
    assert not zero.is_vector()
    assert not zero.is_list()
    assert not zero.is_callable()
    assert zero

    with pytest.raises(TypeError):
        e.float('alpha')


def test_string():
    alpha = e.str('alpha')
    assert e.string_equal(alpha, e.str('alpha'))
    assert repr(alpha) == '"alpha"'
    assert str(alpha) == 'alpha'
    with pytest.raises(ValueError):
        int(alpha)
    with pytest.raises(ValueError):
        float(alpha)
    assert alpha.type() == 'string'
    assert alpha.is_a('string')
    assert not alpha.is_int()
    assert not alpha.is_float()
    assert alpha.is_str()
    assert not alpha.is_symbol()
    assert not alpha.is_cons()
    assert not alpha.is_vector()
    assert not alpha.is_list()
    assert not alpha.is_callable()
    assert alpha

    with pytest.raises(TypeError):
        e.str(2)
    with pytest.raises(TypeError):
        e.str(1.1)

    s_one = e.str('1')
    i_one = int(s_one)
    assert i_one == 1

    s_two = e.str('2.2')
    f_two = float(s_two)
    assert f_two == 2.2


def test_cons():
    cons = e.intern('cons')
    assert cons.is_callable()
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')
    nil = e.intern('nil')

    with pytest.raises(ValueError):
        cons(kwd='value')

    cell = cons(a, b)
    lst = cons(a, cons(b, cons(c, nil)))

    assert e.equal(cell, cons(a, b))
    assert not e.eq(cell, cons(a, b))
    assert repr(cell) == '(a . b)'
    assert str(cell) == '(a . b)'
    with pytest.raises(TypeError):
        int(cell)
    with pytest.raises(TypeError):
        float(cell)
    assert cell.type() == 'cons'
    assert cell.is_a('cons')
    assert not cell.is_int()
    assert not cell.is_float()
    assert not cell.is_str()
    assert not cell.is_symbol()
    assert cell.is_cons()
    assert not cell.is_vector()
    assert cell.is_list()
    assert not cell.is_callable()
    assert cell

    assert e.equal(lst, cons(a, cons(b, cons(c, nil))))
    assert not e.eq(lst, cons(a, cons(b, cons(c, nil))))
    assert repr(lst) == '(a b c)'
    assert str(lst) == '(a b c)'
    with pytest.raises(TypeError):
        int(lst)
    with pytest.raises(TypeError):
        float(lst)
    assert lst.type() == 'cons'
    assert lst.is_a('cons')
    assert not lst.is_int()
    assert not lst.is_float()
    assert not lst.is_str()
    assert not lst.is_symbol()
    assert lst.is_cons()
    assert not lst.is_vector()
    assert lst.is_list()
    assert not lst.is_callable()
    assert lst


def test_vector():
    vector = e.intern('vector')
    assert vector.is_callable()
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')

    vec = vector(a, b, c)

    assert e.equal(vec, vector(a, b, c))
    assert not e.eq(vec, vector(a, b, c))
    assert repr(vec) == '[a b c]'
    assert str(vec) == '[a b c]'
    with pytest.raises(TypeError):
        int(vec)
    with pytest.raises(TypeError):
        float(vec)
    assert vec.type() == 'vector'
    assert vec.is_a('vector')
    assert not vec.is_int()
    assert not vec.is_float()
    assert not vec.is_str()
    assert not vec.is_symbol()
    assert not vec.is_cons()
    assert vec.is_vector()
    assert not vec.is_list()
    assert not vec.is_callable()
    assert vec


def test_list():
    list = e.intern('list')
    assert list.is_callable()
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')

    lst = list(a, b, c)

    assert e.equal(lst, list(a, b, c))
    assert not e.eq(lst, list(a, b, c))
    assert repr(lst) == '(a b c)'
    assert str(lst) == '(a b c)'
    with pytest.raises(TypeError):
        int(lst)
    with pytest.raises(TypeError):
        float(lst)
    assert lst.type() == 'cons'
    assert lst.is_a('cons')
    assert not lst.is_int()
    assert not lst.is_float()
    assert not lst.is_str()
    assert not lst.is_symbol()
    assert lst.is_cons()
    assert not lst.is_vector()
    assert lst.is_list()
    assert not lst.is_callable()
    assert lst


def test_function():
    def a():
        return e.int(1)
    func = e.function(a, 0, 0)
    assert func.is_callable()
    ret = func()
    assert str(ret) == '1'
    assert ret.is_int()
    assert int(ret) == 1

    def b():
        return e.str('alpha')
    func = e.function(b, 0, 0)
    assert func.is_callable()
    ret = func()
    assert ret.is_str()
    assert str(ret) == 'alpha'


def test_error():
    list = e.intern('list')

    error = e.intern('error')
    msg = e.str('An error message')
    with pytest.raises(e.Signal) as ex:
        error(msg)
    sym, data = ex.value.args
    assert e.eq(sym, e.intern('error'))
    assert e.equal(data, list(msg))

    throw = e.intern('throw')
    tag = e.intern('tag')
    value = e.int(1)
    with pytest.raises(e.Throw) as ex:
        throw(tag, value)
    sym, data = ex.value.args
    assert e.eq(sym, e.intern('tag'))
    assert e.equal_sign(data, value)

    def err():
        error = e.intern('error')
        list = e.intern('list')
        raise e.Signal(error, list(e.str('message')))
    func = e.function(err, 0, 0)
    with pytest.raises(e.Signal) as ex:
        func()
    sym, data = ex.value.args
    assert e.eq(sym, e.intern('error'))
    assert e.equal(data, list(e.str('message')))


def test_cmp():
    assert e.int(0) == e.int(0)
    assert e.int(0) != e.int(1)
    assert e.int(0) < e.int(1)
    assert not e.int(1) <= e.int(0)

    assert e.int(0) == 0
    assert 0 == e.int(0)
    assert 1 < e.int(2)

    assert e.float(9.1) >= 9.1
    assert e.float(8) == 8
    assert 0 == e.float(0.0)

    assert 'a' == e.str('a')
    assert e.str('abc') < 'abd'
    assert 'q' != e.str('Q')

    assert not e.str('a') == []
    assert e.str('a') != []

    with pytest.raises(TypeError):
        e.str('a') < 1
    with pytest.raises(TypeError):
        e.str('a') > []
    with pytest.raises(TypeError):
        e.str('a') >= ()


def test_num():
    assert e.int(0) + e.int(1) == 1
    assert e.int(3) + 8 == 11
    assert 7 + e.float(-3.1) == 3.9
    assert 'alpha' + e.str('bravo') == 'alphabravo'

    assert e.int(8) - e.int(1) == 7
    assert 13 - e.float(2.2) == 10.8

    assert e.int(5) * e.float(2.2) == 11.0
    assert 7 * e.int(2) == 14

    assert e.int(17) // 3 == 5
    assert 7 / e.int(2) == 3.5
    assert e.int(8) / e.int(-1) == -8.0

    assert divmod(e.int(10), e.int(3)) == (3, 1)

    assert 2 ** e.int(3) == 8
    assert e.float(3.0) ** e.int(3) == 27.0
    assert pow(e.int(2), e.int(4), e.int(5)) == 1

    assert -e.int(2) == -2
    assert -e.float(-2.2) == 2.2

    assert +e.int(-2) == -2
    assert ~e.int(1) == ~1

    assert (e.int(8) << 1) == 16
    assert 8 >> e.int(1) == 4

    with pytest.raises(TypeError):
        13 - e.str('a')
    with pytest.raises(TypeError):
        e.intern('a') + 2
    with pytest.raises(TypeError):
        2 << e.intern('b')
    with pytest.raises(TypeError):
        pow(1, 2, e.intern('q'))
