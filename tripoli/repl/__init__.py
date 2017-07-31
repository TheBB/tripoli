import code


def run_repl():
    c = code.InteractiveConsole()
    c.push('import tripoli as t')
    c.push('import emacs_raw as er')
    c.push('import emacs as e')
    c.interact()
