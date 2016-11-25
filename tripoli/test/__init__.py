from os.path import abspath, dirname, join
import pytest
import sys


def run_tests(args):
    try:
        # Check that we are running embedded
        import emacs_raw
    except ImportError:
        print("Can't run tests outside Emacs embedding.")
    else:

        try:
            # Fool pytest into running
            sys.argv = ['tripoli']

            module = abspath(join(dirname(__file__), '..'))
            pytest.main(args + [module])

        except Exception as e:
            print('{}: {}'.format(e.__class__.__name__, e))
            _, _, tb = sys.exc_info()
            traceback.print_tb(tb)
