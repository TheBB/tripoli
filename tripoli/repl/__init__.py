import code


def run_repl():
    c = code.InteractiveConsole()
    c.push('import emacs_raw as er')
    c.interact()
