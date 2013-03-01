#!/bin/sh
#

make distclean

if ! ./configure.windows; then
	exit 1
fi

if ! make distwindows distclean; then
	exit 1
fi

if ! ./configure.dos; then
	exit 1
fi

if ! make distdos distclean; then
	exit 1
fi

if ! ./configure ; then
	exit 1
fi

if ! make distcheck; then
	exit 1
fi

