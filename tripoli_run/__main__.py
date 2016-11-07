from subprocess import run
from os.path import abspath, dirname, join
import os

def main():
    tripoli_path = os.environ.get('TRIPOLI_PATH')
    if not tripoli_path:
        tripoli_path = abspath(join(dirname(__file__), '..', 'build'))
    run(['emacs', '-q', '-L', tripoli_path, '-l', 'libtripoli',
         '--eval', '(tripoli-import "tripoli")'])

if __name__ == '__main__':
    main()
