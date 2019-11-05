#!/bin/bash

. $(dirname $(readlink -f $0))/paths

cd $OBJS && \
make -j `nproc` && make install && \
cd $SRCDIR
