import emacs
import pytest

def test_nil():
    nil = emacs.intern('nil')
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
    assert not nil

def test_intern():
    alpha = emacs.intern('alpha')
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
    assert alpha

    with pytest.raises(TypeError):
        emacs.intern(2)
    with pytest.raises(TypeError):
        emacs.intern([])

def test_int():
    one = emacs.int(1)
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
    assert one

    zero = emacs.int(0)
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
    assert zero

    with pytest.raises(TypeError):
        emacs.int(0.1)
    with pytest.raises(TypeError):
        emacs.int('alpha')

def test_float():
    one = emacs.float(1.1)
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
    assert one

    zero = emacs.float(0)
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
    assert zero

    with pytest.raises(TypeError):
        emacs.float('alpha')

def test_string():
    alpha = emacs.str('alpha')
    assert repr(alpha) == '"alpha"'
    assert str(alpha) == 'alpha'
    with pytest.raises(TypeError):
        int(alpha)
    with pytest.raises(TypeError):
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
    assert alpha

    with pytest.raises(TypeError):
        emacs.str(2)
    with pytest.raises(TypeError):
        emacs.str(1.1)

def test_cons():
    cons = emacs.intern('cons')
    a = emacs.intern('a')
    b = emacs.intern('b')
    c = emacs.intern('c')
    nil = emacs.intern('nil')

    cell = cons(a, b)
    lst = cons(a, cons(b, cons(c, nil)))

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
    assert cell

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
    assert lst

def test_vector():
    vector = emacs.intern('vector')
    a = emacs.intern('a')
    b = emacs.intern('b')
    c = emacs.intern('c')

    vec = vector(a, b, c)

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
    assert vec

def test_list():
    list = emacs.intern('list')
    a = emacs.intern('a')
    b = emacs.intern('b')
    c = emacs.intern('c')

    lst = list(a, b, c)

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
    assert lst

def test_function():
    def a():
        return emacs.int(1)
    func = emacs.function(a, 0, 0)
    ret = func()
    assert ret.is_int()
    assert int(ret) == 1

    def b():
        return emacs.str('alpha')
    func = emacs.function(b, 0, 0)
    ret = func()
    assert ret.is_str()

    # Fails. Why?
    # assert str(ret) == 'alpha'
