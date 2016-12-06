from tripoli import require
from tripoli.decorators import add_hook, defun, eval_after_load
from tripoli.types import AssociationList, List
from emacs import package, message

@eval_after_load('package')
def load_check():
    message('package was loaded!')

require('package')

archives = AssociationList(package.archives, inplace=True)
archives.clear()
archives.update({
    'gnu': 'https://elpa.gnu.org/packages/',
    'org': 'http://orgmode.org/elpa/',
    'melpa': 'https://melpa.org/packages/',
})

package.initialize()

@defun('some-function', interactive='f')
def test(arg):
    message(arg)
    return 63

@add_hook('emacs-lisp-mode-hook', name='myfun')
def lmfao():
    message('lmfao')
