from subprocess import run
from os.path import abspath, dirname, join
import os
import sys

def main():
    tripoli_path = os.environ.get('TRIPOLI_PATH')
    if not tripoli_path:
        tripoli_path = abspath(join(dirname(__file__), '..', 'build'))
    args = ['emacs', '-q', '-L', tripoli_path, '-l', 'libtripoli']
    if '--test' in sys.argv:
        args.insert(1, '--batch')
        args.extend(['--eval', '(tripoli-import "tripoli_test")',
                     '--eval', '(kill-emacs)'])
    run(args, check=True)

if __name__ == '__main__':
    main()
