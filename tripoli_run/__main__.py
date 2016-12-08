import glob
from os.path import abspath, dirname, join
import os
import subprocess
import sys
import textwrap

import click


def emacs_escape(s):
    return s.replace('\\', r'\\').replace('"', r'\"')


@click.group(invoke_without_command=True)
@click.option('--dev/--no-dev', default=False)
@click.pass_context
def main(ctx, dev):
    path = os.environ.get('TRIPOLI_PATH')
    if dev and not path:
        root = abspath(join(dirname(__file__), '..'))
        try:
            path = dirname(max(
                glob.iglob(join(root, '**', 'libtripoli.so'), recursive=True),
                key=os.path.getmtime
            ))
        except ValueError:
            pass
    elif not dev and not path:
        candidates = [
            '/usr/lib/libtripoli.so',
            '/usr/local/lib/libtripoli.so',
            '/lib/libtripoli.so',
        ]
        for c in candidates:
            if os.path.exists(c) and os.path.isfile(c):
                path = dirname(c)
                break

    if not path:
        print("Could not find libtripoli.", file=sys.stderr)
        sys.exit(1)

    ctx.obj = {
        'path': path,
        'args': ['emacs', '-q', '-L', path, '-l', 'libtripoli']
    }
    if ctx.invoked_subcommand is None:
        ctx.invoke(run)


@main.command()
@click.pass_context
def run(ctx):
    args = ctx.obj['args']
    dotfile = join(dirname(__file__), '..', 'dotemacs.py')
    args.extend(['--eval', '(tripoli-run-file "{}")'.format(dotfile)])
    subprocess.run(args, check=True)


@main.command(context_settings={'ignore_unknown_options': True})
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def test(ctx, extra_args):
    test_code = textwrap.dedent("""
    from tripoli.test import run_tests
    run_tests([{}])
    """)
    test_code = test_code.format(','.join(emacs_escape(repr(c)) for c in extra_args))
    args = ctx.obj['args']
    args.insert(1, '--batch')
    args.extend(['--eval', '(tripoli-run-string "{}")'.format(test_code),
                 '--eval', '(kill-emacs)'])
    subprocess.run(args, check=True)


@main.command()
@click.pass_context
def repl(ctx):
    repl_code = textwrap.dedent("""
    import code
    c = code.InteractiveConsole()
    c.push('import tripoli')
    c.push('import emacs_raw as er')
    c.push('import emacs as e')
    c.interact()
    """)
    args = ctx.obj['args']
    args.insert(1, '--batch')
    args.extend(['--eval', '(tripoli-run-string "{}")'.format(repl_code)])
    subprocess.run(args, check=True)


if __name__ == '__main__':
    main()
