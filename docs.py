#!/usr/bin/env python3

import subprocess
import textwrap
import os
from os.path import join


if __name__ == '__main__':
    cwd = os.getcwd()
    docdir = join(cwd, '..', 'docs')

    code = textwrap.dedent("""
    import sys
    sys.argv = []
    from sphinx import main
    sys.exit(main(['sphinx-build', '-b', 'html', '-d', 'build/doctrees', 'source', 'build/html']))
    """)
    args = [
        'emacs', '-q', '--batch', '-L', cwd,
        '--eval', '(setq tripoli-inhibit-init t)',
        '-l', 'libtripoli',
        '--eval', '(tripoli-exec-str "{}")'.format(code)
    ]
    subprocess.run(args, check=True, cwd=docdir)
