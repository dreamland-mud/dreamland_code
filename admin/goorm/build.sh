#!/bin/bash

. $(dirname $(readlink -f $0))/paths

cd $OBJS && \
make -j 2 && make install && \
cd $SRCDIR
