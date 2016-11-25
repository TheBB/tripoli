from tripoli import require
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
