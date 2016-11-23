from tripoli import require
from tripoli.types import AssociationList, List

import emacs_raw

import emacs as e
from emacs import cons

features = List(e.features)

require('package')

archives = AssociationList(e.package_archives)
archives['gnu'] = 'https://elpa.gnu.org/packages/'
archives['org'] = 'https://orgmode.org/packages/'
print(archives.place)
del archives['gnu']
print(archives.place)
