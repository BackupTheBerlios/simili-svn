#!/bin/sh

echo autoconf / automake is broken and slow,
echo just type 'make install'

#exit 0

aclocal
libtoolize
automake --add-missing -a -c
autoconf
./configure
