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
    assert not e.integerp(nil)
    assert not e.floatp(nil)
    assert not e.stringp(nil)
    assert e.symbolp(nil)
    assert not e.consp(nil)
    assert not e.vectorp(nil)
    assert e.listp(nil)
    assert not e.functionp(nil)
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
    assert not e.integerp(alpha)
    assert not e.floatp(alpha)
    assert not e.stringp(alpha)
    assert e.symbolp(alpha)
    assert not e.consp(alpha)
    assert not e.vectorp(alpha)
    assert not e.listp(alpha)
    assert not e.functionp(alpha)
    assert alpha

    with pytest.raises(TypeError):
        e.intern(2)
    with pytest.raises(TypeError):
        e.intern([])


def test_int():
    one = e.int('1')
    assert e.eq(one, e.int(1))
    assert repr(one) == '1'
    assert str(one) == '1'
    assert int(one) == 1
    assert float(one) == 1.0
    assert one.type() == 'integer'
    assert one.is_a('integer')
    assert e.integerp(one)
    assert not e.floatp(one)
    assert not e.stringp(one)
    assert not e.symbolp(one)
    assert not e.consp(one)
    assert not e.vectorp(one)
    assert not e.listp(one)
    assert not e.functionp(one)
    assert one

    zero = e.int(0)
    assert e.eq(zero, e.int(0))
    assert repr(zero) == '0'
    assert str(zero) == '0'
    assert int(zero) == 0
    assert float(zero) == 0.0
    assert zero.type() == 'integer'
    assert zero.is_a('integer')
    assert e.integerp(zero)
    assert not e.floatp(zero)
    assert not e.stringp(zero)
    assert not e.symbolp(zero)
    assert not e.consp(zero)
    assert not e.vectorp(zero)
    assert not e.listp(zero)
    assert not e.functionp(zero)
    assert zero

    assert str(e.int(0.1)) == '0'
    with pytest.raises(ValueError):
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
    assert not e.integerp(one)
    assert e.floatp(one)
    assert not e.stringp(one)
    assert not e.symbolp(one)
    assert not e.consp(one)
    assert not e.vectorp(one)
    assert not e.listp(one)
    assert not e.functionp(one)
    assert one

    zero = e.float('0')
    assert e.eql(zero, e.float(0))
    assert repr(zero) == '0.0'
    assert str(zero) == '0.0'
    assert int(zero) == 0
    assert float(zero) == 0.0
    assert zero.type() == 'float'
    assert zero.is_a('float')
    assert not e.integerp(zero)
    assert e.floatp(zero)
    assert not e.stringp(zero)
    assert not e.symbolp(zero)
    assert not e.consp(zero)
    assert not e.vectorp(zero)
    assert not e.listp(zero)
    assert not e.functionp(zero)
    assert zero

    with pytest.raises(ValueError):
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
    assert not e.integerp(alpha)
    assert not e.floatp(alpha)
    assert e.stringp(alpha)
    assert not e.symbolp(alpha)
    assert not e.consp(alpha)
    assert not e.vectorp(alpha)
    assert not e.listp(alpha)
    assert not e.functionp(alpha)
    assert alpha

    assert str(e.str(2)) == '2'
    assert str(e.str(1.1)) == '1.1'

    s_one = e.str('1')
    i_one = int(s_one)
    assert i_one == 1

    s_two = e.str('2.2')
    f_two = float(s_two)
    assert f_two == 2.2


def test_cons_ctr():
    a = e.intern('a')
    b = e.intern('b')

    assert repr(e.cons(a, b)) == '(a . b)'
    assert repr(e.cons(a)) == '(a)'
    assert repr(e.cons()) == 'nil'


def test_list_ctr():
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')

    assert repr(e.list([a, b, c])) == '(a b c)'
    assert repr(e.list((a, b, c))) == '(a b c)'
    assert repr(e.list(iter([a, b, c]))) == '(a b c)'
    assert repr(e.list([])) == 'nil'
    assert repr(e.list()) == 'nil'
    assert repr(e.list([a, b, c])[0]) == 'a'
    assert repr(e.list([a, b, c])[-1]) == 'c'

    with pytest.raises(TypeError):
        a[0]

    with pytest.raises(IndexError):
        e.list()[0]


def test_vector_ctr():
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')

    assert repr(e.vector([a, b, c])) == '[a b c]'
    assert repr(e.vector((a, b, c))) == '[a b c]'
    assert repr(e.vector(iter([a, b, c]))) == '[a b c]'
    assert repr(e.vector([])) == '[]'
    assert repr(e.vector()) == '[]'
    assert repr(e.vector([a, b, c])[0]) == 'a'
    assert repr(e.vector([a, b, c])[-1]) == 'c'

    with pytest.raises(IndexError):
        e.vector()[0]


def test_length():
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')

    assert len(e.list([a, b, c])) == 3
    assert len(e.vector([a, b])) == 2
    assert len(e.str('alpha')) == 5
    assert len(e.intern('nil')) == 0

    with pytest.raises(TypeError):
        len(e.cons(a, b))


def test_cons():
    cons = e.intern('cons')
    assert e.functionp(cons)
    a = e.intern('a')
    b = e.intern('b')
    c = e.intern('c')
    nil = e.intern('nil')

    assert repr(cons(kwd='value')) == '(:kwd . "value")'

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
    assert not e.integerp(cell)
    assert not e.floatp(cell)
    assert not e.stringp(cell)
    assert not e.symbolp(cell)
    assert e.consp(cell)
    assert not e.vectorp(cell)
    assert e.listp(cell)
    assert not e.functionp(cell)
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
    assert not e.integerp(lst)
    assert not e.floatp(lst)
    assert not e.stringp(lst)
    assert not e.symbolp(lst)
    assert e.consp(lst)
    assert not e.vectorp(lst)
    assert e.listp(lst)
    assert not e.functionp(lst)
    assert lst


def test_vector():
    vector = e.intern('vector')
    assert e.functionp(vector)
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
    assert not e.integerp(vec)
    assert not e.floatp(vec)
    assert not e.stringp(vec)
    assert not e.symbolp(vec)
    assert not e.consp(vec)
    assert e.vectorp(vec)
    assert not e.listp(vec)
    assert not e.functionp(vec)
    assert vec


def test_list():
    list = e.intern('list')
    assert e.functionp(list)
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
    assert not e.integerp(lst)
    assert not e.floatp(lst)
    assert not e.stringp(lst)
    assert not e.symbolp(lst)
    assert e.consp(lst)
    assert not e.vectorp(lst)
    assert e.listp(lst)
    assert not e.functionp(lst)
    assert lst


def test_function():
    def a():
        return e.int(1)
    func = e.function(a, 0, 0)
    assert e.functionp(func)
    ret = func()
    assert e.eq(ret, e.int(1))

    def b():
        return e.str('alpha')
    func = e.function(b)
    assert e.functionp(func)
    ret = func()
    assert e.string_equal(ret, e.str('alpha'))


def test_compare():
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
