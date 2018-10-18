#!/bin/bash

. $(dirname $(readlink -f $0))/paths

mkdir -p $OBJS && \
cd $SRCDIR && \
make -f Makefile.git && \
cd $OBJS && \
$SRCDIR/configure --prefix=$RUNTIME 

