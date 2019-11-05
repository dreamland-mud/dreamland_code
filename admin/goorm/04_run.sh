#!/bin/bash

. $(dirname $(readlink -f $0))/paths

cd $RUNTIME && \
./bin/dreamland etc/dreamland.xml &
