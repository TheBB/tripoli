from os.path import abspath, dirname, join
import os
import subprocess
import sys
import textwrap

import click


@click.group(invoke_without_command=True)
@click.pass_context
def main(ctx):
    path = os.environ.get('TRIPOLI_PATH') or abspath(join(dirname(__file__), '..', 'build'))
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

    # The fingers-crossed method of string escaping
    test_code = test_code.format(','.join("'{}'".format(c) for c in extra_args))

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
