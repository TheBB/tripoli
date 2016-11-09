import emacs
import sys
import traceback

def hello():
    print('hello')

try:
    functionp = emacs.intern('functionp')
    assert functionp(emacs.intern('intern'))
    assert not functionp(emacs.intern('nil'))

    cons = emacs.intern('cons')
    a = emacs.intern('a')
    b = emacs.intern('b')
    obj = cons(a, b)
    assert repr(obj) == '(a . b)'
    assert obj.type() == 'cons'
    assert obj.is_a('cons')
    assert obj.is_cons()
    assert obj.is_list()

    message = emacs.intern('message')
    string = emacs.str('Tripoli successfully loaded')
    message(string)

    func = emacs.function(hello, 0, 0)
    hello = emacs.intern('hello')
    fset = emacs.intern('fset')
    fset(hello, func)

    s = emacs.str('hello')
    assert s.type() == 'string'
    assert s.is_a('string')
    assert s.is_str()
    assert str(s) == 'hello'
    assert repr(s) == '"hello"'

    i = emacs.int(2)
    assert str(i) == '2'
    assert i.type() == 'integer'
    assert i.is_a('integer')
    assert i.is_int()
    assert int(i) == 2

    f = emacs.float(2.2)
    assert str(f) == '2.2'
    assert f.type() == 'float'
    assert f.is_a('float')
    assert f.is_float()
    assert not f.is_cons()
    assert int(f) == 2

    print('Tripoli successfully loaded')
except Exception as e:
    print('{}: {}'.format(e.__class__.__name__, e))
    _, _, tb = sys.exc_info()
    traceback.print_tb(tb)
