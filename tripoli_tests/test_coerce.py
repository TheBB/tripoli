from tripoli.util import coerce

import emacs_raw as er


def test_all():

    @coerce(True)
    def test(a, b, c, *args, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert isinstance(b, er.EmacsObject)
        assert isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert isinstance(kwarg, er.EmacsObject)

    test(1, 2, 3)
    test('a', [34], er.intern('q'))
    test('a', [34], er.intern('q'), 'b', 'c', q='d')


def test_none():

    @coerce(False)
    def test(a, b, c, *args, **kwargs):
        assert not isinstance(a, er.EmacsObject)
        assert not isinstance(b, er.EmacsObject)
        assert not isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert not isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert not isinstance(kwarg, er.EmacsObject)

    test(1, 2, 3)
    test('a', [34], 'q')
    test('a', [34], 'q', 'b', 'c', q='d')


def test_none_but_passed():

    @coerce(False)
    def test(a, b, c, *args, **kwargs):
        assert not isinstance(a, er.EmacsObject)
        assert not isinstance(b, er.EmacsObject)
        assert isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert not isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert not isinstance(kwarg, er.EmacsObject)

    test(1, 2, er.int(3))
    test('a', [34], er.intern('q'))
    test('a', [34], er.intern('q'), 'b', 'c', q='d')


def test_almost_all():

    @coerce(True, ['b'])
    def test(a, b, c, *args, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert not isinstance(b, er.EmacsObject)
        assert isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert isinstance(kwarg, er.EmacsObject)

    test(1, 2, 3)
    test('a', [34], er.intern('q'))
    test('a', [34], er.intern('q'), 'b', 'c', q='d')


def test_almost_all_but_passed():

    @coerce(True, ['c'])
    def test(a, b, c, *args, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert isinstance(b, er.EmacsObject)
        assert isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert isinstance(kwarg, er.EmacsObject)

    test(1, 2, er.int(3))
    test('a', [34], er.intern('q'))
    test('a', [34], er.intern('q'), 'b', 'c', q='d')


def test_almost_none():

    @coerce(False, ['a'])
    def test(a, b, c, *args, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert not isinstance(b, er.EmacsObject)
        assert not isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert not isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert not isinstance(kwarg, er.EmacsObject)

    test(1, 2, 3)
    test('a', [34], 'q')
    test('a', [34], 'q', 'b', 'c', q='d')


def test_almost_none_but_passed():

    @coerce(False, ['a'])
    def test(a, b, c, *args, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert not isinstance(b, er.EmacsObject)
        assert isinstance(c, er.EmacsObject)

        assert not isinstance(args, er.EmacsObject)
        assert not isinstance(kwargs, er.EmacsObject)

        for arg in args:
            assert not isinstance(arg, er.EmacsObject)
        for name, kwarg in kwargs.items():
            assert not isinstance(name, er.EmacsObject)
            assert not isinstance(kwarg, er.EmacsObject)

    test(1, 2, er.int(3))
    test('a', [34], er.intern('q'))
    test('a', [34], er.intern('q'), 'b', 'c', q='d')


def test_avoid_args():

    @coerce(True, ['args'])
    def test(a, b, *args):
        assert isinstance(a, er.EmacsObject)
        assert isinstance(b, er.EmacsObject)
        for arg in args:
            assert not isinstance(arg, er.EmacsObject)

    test('a', 'b', 'c', 'd')


def test_avoid_kwargs():

    @coerce(True, ['kwargs'])
    def test(a, b, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert isinstance(b, er.EmacsObject)
        for kwarg in kwargs.values():
            assert not isinstance(kwarg, er.EmacsObject)

    test('a', 'b', c='c', d='d')


def test_prefer_symbol():

    @coerce(True, (), True)
    def test(a):
        assert er.symbolp(a)

    test('a')
    test(er.intern('a'))


def test_prefer_symbol_almost_all():

    @coerce(True, (), True, ('b'))
    def test(a, b, c):
        assert er.symbolp(a)
        assert not er.symbolp(b)
        assert er.symbolp(c)

    test('a', 'b', 'c')
    test(er.intern('a'), 'b', 'c')


def test_prefer_symbol_almost_none():

    @coerce(True, (), False, ('b'))
    def test(a, b, c):
        assert not er.symbolp(a)
        assert er.symbolp(b)
        assert not er.symbolp(c)

    test('a', 'b', 'c')
    test('a', er.intern('b'), 'c')


def test_prefer_symbol_none_but_passed():

    @coerce(True, (), False)
    def test(a, b, c):
        assert not er.symbolp(a)
        assert er.symbolp(b)
        assert not er.symbolp(c)

    test('a', er.intern('b'), 'c')


def test_prefer_symbol_self():

    class TestSelf: pass

    yes = TestSelf()
    yes.prefer_symbol = True

    no = TestSelf()
    no.prefer_symbol = False

    @coerce(True, (), 'prefer_symbol')
    def test(self, a):
        assert er.symbolp(a) is self.prefer_symbol

    test(yes, 'a')
    test(no, 'b')
