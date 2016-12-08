import glob
from os.path import abspath, dirname, exists, expanduser, getmtime, isfile, join
import os
import subprocess
import sys
import textwrap

import click


def emacs_escape(s):
    return s.replace('\\', r'\\').replace('"', r'\"')


@click.group(
    invoke_without_command=True,
    context_settings={'ignore_unknown_options': True}
)
@click.option('--dev/--no-dev', default=False)
@click.argument('extra_args', nargs=-1, type=click.UNPROCESSED)
@click.pass_context
def main(ctx, dev, extra_args):
    path = os.environ.get('TRIPOLI_PATH')
    if dev and not path:
        root = abspath(join(dirname(__file__), '..'))
        try:
            path = dirname(max(
                glob.iglob(join(root, '**', 'libtripoli.so'), recursive=True),
                key=getmtime
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
            if exists(c) and isfile(c):
                path = dirname(c)
                break

    if not path:
        print("Could not find libtripoli.", file=sys.stderr)
        sys.exit(1)

    ctx.obj = {
        'path': path,
        'args': ['emacs', '-L', path, '-l', 'libtripoli'],
    }
    if ctx.invoked_subcommand is None:
        if extra_args and extra_args[0] in main.commands:
            extra_args, cmd = extra_args[1:], main.commands[extra_args[0]]
        else:
            cmd = run
        ctx.obj['extra_args'] = extra_args
        ctx.invoke(cmd)


@main.command()
@click.pass_context
def run(ctx):
    for dotfile in ['~/.emacs.py', '~/.emacs.d/init.py']:
        dotfile = expanduser(dotfile)
        if exists(dotfile) and isfile(dotfile):
            break
    else:
        dotfile = None

    args = ctx.obj['args']
    if dotfile:
        args.extend(['--eval', '(tripoli-run-file "{}")'.format(dotfile)])
    args.extend(ctx.obj['extra_args'])
    subprocess.run(args, check=True)


@main.command()
@click.pass_context
def test(ctx):
    test_code = textwrap.dedent("""
    from tripoli.test import run_tests
    run_tests([{}])
    """)
    test_code = test_code.format(
        ','.join(emacs_escape(repr(c)) for c in ctx.obj['extra_args'])
    )
    args = [ctx.obj['args'][0]] + ['-q', '--batch'] + ctx.obj['args'][1:]
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
    args = [ctx.obj['args'][0]] + ['-q', '--batch'] + ctx.obj['args'][1:]
    args.extend(['--eval', '(tripoli-run-string "{}")'.format(repl_code)])
    subprocess.run(args, check=True)


if __name__ == '__main__':
    main()
