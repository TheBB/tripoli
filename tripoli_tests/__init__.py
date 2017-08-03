from os.path import abspath, dirname, join
import pytest
import sys
import os


def run_tests(*args):
    args = list(args)

    if os.getenv('TESTVERBOSE'):
        args += ['-v']

    coverage = os.getenv('COVERAGE')
    cfg_file = abspath(join(dirname(__file__), '..', 'coverage.cfg'))
    if coverage and os.path.isfile(cfg_file):
        args += ['--cov-config', cfg_file, '--cov=tripoli']
        if coverage in {'html', 'xml', 'annotate'}:
            args += ['--cov-report', coverage]

    module = abspath(join(dirname(__file__)))
    args += [module]

    # Fool pytest into running
    sys.argv = ['tripoli']

    try:
        return pytest.main(args)

    except Exception as e:
        print('{}: {}'.format(e.__class__.__name__, e))
        _, _, tb = sys.exc_info()
        traceback.print_tb(tb)
        return 101
