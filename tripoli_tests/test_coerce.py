from tripoli.util import coerce

import emacs_raw as er


def test_all():

    @coerce()
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


def test_almost_all():

    @coerce('b', invert=True)
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

    @coerce('c', invert=True)
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

    @coerce('a')
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

    @coerce('a')
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

    @coerce('args', invert=True)
    def test(a, b, *args):
        assert isinstance(a, er.EmacsObject)
        assert isinstance(b, er.EmacsObject)
        for arg in args:
            assert not isinstance(arg, er.EmacsObject)

    test('a', 'b', 'c', 'd')


def test_avoid_kwargs():

    @coerce('kwargs', invert=True)
    def test(a, b, **kwargs):
        assert isinstance(a, er.EmacsObject)
        assert isinstance(b, er.EmacsObject)
        for kwarg in kwargs.values():
            assert not isinstance(kwarg, er.EmacsObject)

    test('a', 'b', c='c', d='d')


def test_prefer_symbol():

    @coerce(prefer_symbol=True)
    def test(a):
        assert er.symbolp(a)

    test('a')
    test(er.intern('a'))


def test_prefer_symbol_none_but_passed():

    @coerce(prefer_symbol=False)
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

    @coerce(prefer_symbol='prefer_symbol')
    def test(self, a):
        assert er.symbolp(a) is self.prefer_symbol

    test(yes, 'a')
    test(no, 'b')
