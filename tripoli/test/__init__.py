from os.path import abspath, dirname, join
import pytest
import sys


def run_tests(*args):
    module = abspath(join(dirname(__file__), '..'))
    args = list(args) + [module]

    # Fool pytest into running
    sys.argv = ['tripoli']

    try:
        return pytest.main(args)

    except Exception as e:
        print('{}: {}'.format(e.__class__.__name__, e))
        _, _, tb = sys.exc_info()
        traceback.print_tb(tb)
        return 101
