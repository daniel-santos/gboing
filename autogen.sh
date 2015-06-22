#!/bin/bash

auxdir=autoscripts

set -x

# aclocal generated files
rm -rf aclocal.m4 autom4te.cache

# autoheader generated files
rm -f config.h.in

# autoconf generated files
rm -f configure

# libtoolize generated files
rm -f ${auxdir}/ltmain.sh

# automake generated files
rm -f ${auxdir}/{config.guess,config.sub,depcomp,install-sh,missing}
rm -f {,src/lib/,src/test/}Makefile.in

rm -f $(find ${auxdir} m4 -type l 2>/dev/null)
rmdir ${auxdir} m4 2>/dev/null

if [[ "$1" == clean ]]; then
	exit 0
fi

mkdir ${auxdir} m4 2>/dev/null
touch NEWS ChangeLog

aclocal
autoheader
autoconf
libtoolize
automake --add-missing

