#!/bin/sh

aclocal
automake --foreign
autoconf
touch stamp-h.in

