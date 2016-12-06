from tripoli import require
from tripoli.decorators import defun, add_hook
from tripoli.types import AssociationList, List
from emacs import package, message

require('package')

archives = AssociationList(package.archives, inplace=True)
archives.clear()
archives.update({
    'gnu': 'https://elpa.gnu.org/packages/',
    'org': 'http://orgmode.org/elpa/',
    'melpa': 'https://melpa.org/packages/',
})

package.initialize()

@defun('some-function')
def test():
    return 63

@add_hook('emacs-lisp-mode-hook', name='myfun')
def lmfao():
    message('lmfao')
