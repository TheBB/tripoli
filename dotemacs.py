from tripoli import require
from tripoli.decorators import defun
from tripoli.types import AssociationList, List
from emacs import package

require('package')

archives = AssociationList(package.archives, inplace=True)
archives.clear()
archives.update({
    'gnu': 'https://elpa.gnu.org/packages/',
    'org': 'http://orgmode.org/elpa/',
    'melpa': 'https://melpa.org/packages/',
})

package.initialize()
package.refresh_contents()

@defun('some-function')
def test():
    """Here's a docstring."""
    return 63
