#!/bin/bash

ROOT=/workspace
RUNTIME=$ROOT/runtime

cd $RUNTIME && \
./bin/dreamland etc/dreamland.xml &
