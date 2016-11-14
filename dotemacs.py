import tripoli

import emacs as e
from emacs import cons

e.require(e.package)

e.package_archives = e.list(
    cons('melpa', 'https://melpa.org/packages/'),
    cons('org', 'https://orgmode.org/packages/'),
    cons('gnu', 'https://elpa.gnu.org/packages/'),
)

e.package_initialize()
