import IPython


def run_repl():
    modules = [
        ('tripoli', 't'),
        ('emacs_raw', 'er'),
        ('emacs', 'e'),
    ]
    code = ', '.join('"import {} as {}"'.format(a, b) for a, b in modules)
    IPython.start_ipython(argv=[
        '--HistoryManager.enabled=False',
        '--InteractiveShellApp.exec_lines=[{}]'.format(code),
    ])
