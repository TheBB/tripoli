#!/usr/bin/env python3

from distutils.core import setup

setup(
    name='Tripoli',
    version='0.2',
    description='Emacs pythonified',
    author='Eivind Fonn',
    author_email='evfonn@gmail.com',
    license='GPL3',
    url='https://github.com/TheBB/tripoli',
    packages=['tripoli', 'tripoli_tests'],
    install_requires=[
        'pytest',
        'sphinx',
        'ipython',
    ],
)
