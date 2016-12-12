================
Building Tripoli
================

First, make sure you have the required dependencies:

- Python 3 is required, probably at least Python 3.4.
- You need to have Emacs installed, and it must be compiled with the
  :code:`--with-modules` option. You should see :code:`MODULES` in your
  :code:`system-configuration-features` variable. Note that this feature is
  experimental and usually not enabled by default in distribution packages.
- You also need the Emacs source code available, for the module header file.
- The shared library build system uses CMake. Pip is recommended for installing
  the Python modules.

Then build the extension module.

.. code:: bash

   mkdir build
   cd build
   cmake ..
   make

You probably need to specify that path to your Emacs source code manually by
providing CMake with a :code:`-DCMAKE_INCLUDE_PATH=…` argument, so that the
:code:`emacs-module.h` header file can be found.

.. code:: bash

   cmake -DCMAKE_INCLUDE_PATH=… ..

Once the library is built, you can install it.

.. code:: bash

   make install

To install the Python modules, do

.. code:: bash

   pip install --user .
