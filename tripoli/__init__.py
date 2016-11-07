import emacs

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
    assert str(obj) == '(a . b)'

    message = emacs.intern('message')
    string = emacs.str('Tripoli successfully loaded')
    message(string)

    func = emacs.make_function(hello, 0, 0)
    hello = emacs.intern('hello')
    fset = emacs.intern('fset')
    fset(hello, func)
except Exception as e:
    print('{}: {}'.format(e.__class__.__name__, e))

print('Tripoli successfully loaded')
