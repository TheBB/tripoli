import glob
from os.path import abspath, dirname, exists, expanduser, getmtime, isfile, join
import os
import subprocess
import sys
import textwrap

import click


@click.command(context_settings={'ignore_unknown_options': True})
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
def main(extra_args):
    path = os.environ.get('TRIPOLI_PATH')
    if not path:
        candidates = [
            '/usr/lib/libtripoli.so',
            '/usr/local/lib/libtripoli.so',
            '/lib/libtripoli.so',
        ]
        for c in candidates:
            if exists(c) and isfile(c):
                path = dirname(c)
                break

    if not path:
        print("Could not find libtripoli.", file=sys.stderr)
        sys.exit(1)

    args = ['emacs', '-L', path, '-l', 'libtripoli']

    dotfile = None
    for dotfile in ['~/.emacs.py', '~/.emacs.d/init.py']:
        dotfile = expanduser(dotfile)
        if exists(dotfile) and isfile(dotfile):
            break
    else:
        dotfile = None
    if dotfile:
        args.extend(['--eval', '(tripoli-run-file "{}")'.format(dotfile)])

    args.extend(extra_args)
    subprocess.run(args, check=True)


if __name__ == '__main__':
    main()
