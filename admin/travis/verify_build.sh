#!/bin/bash

ROOT=/home/travis/build/dreamland-mud
RUNTIME=$ROOT/runtime
OBJS=$ROOT/objs
SRC=$TRAVIS_BUILD_DIR

run_build() {
    make -f Makefile.git && \
    set && \
    mkdir -p $OBJS && \
    cd $OBJS && \
    ../dreamland_code/configure --prefix=$RUNTIME --disable-dependency-tracking && \
    find $ROOT && \
    make -j 2 && make install
}

run_smoke_test() {
    cd $RUNTIME && \
    git clone https://github.com/dreamland-mud/dreamland_world.git share/DL && \
    ./bin/dreamland $SRC/admin/travis/dreamland.xml 
}

travis_script() {
    run_build && run_smoke_test
}

set -e # stop on a non-zero exit code
set -x # display expanded commands

$1;

