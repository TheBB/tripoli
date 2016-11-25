#!/usr/bin/env python3

from distutils.core import setup

setup(
    name='Tripoli',
    version='0.1',
    description='Emacs pythonified',
    author='Eivind Fonn',
    author_email='evfonn@gmail.com',
    license='GPL3',
    url='https://github.com/TheBB/tripoli',
    py_modules=['tripoli_test', 'tripoli_run'],
    entry_points={
        'console_scripts': ['tripoli=tripoli_run.__main__:main'],
    },
    install_requires=[
        'click',
    ],
)
