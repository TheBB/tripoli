# Tripoli

## Dependencies

- Python 3.2+
- The standard build/installation system uses CMake and Pip
- Emacs must be installed, and compiled with the `--with-modules` option
- The Emacs source code must be available to provide a header file

## Installation

First, build the Emacs module.

```shell
mkdir build
cd build
cmake ..
make
```

You probably need to specify the path to your Emacs source code manually by
providing cmake with a `-DCMAKE_INCLUDE_PATH=...` argument, so that the
`emacs-module.h` header file can be found.

Second, install the python modules.

```shell
pip install --user --editable .
```

This should install an executable called `tripoli`. You may have to augment your
`PATH` for it to be discoverable (typically it may be found in `~/.local/bin`).

The `tripoli` executable looks for the Emacs module in the `build` folder by
default. You may change this by setting the `TRIPOLI_PATH` environment variable.
