#!/bin/bash

ROOT=`pwd` # normally /home/travis/build/dreamland-mud/dreamland_code

run_build() {
    mkdir -p objs && \
    make -f Makefile.git && \
    cd objs && \
    ../configure --prefix=$ROOT && \
    make -j 2 && make install 
}

run_smoke_test() {
    mkdir -p share && \
    cd share && \
    git clone https://github.com/dreamland-mud/dreamland_world.git DL && \
    cd $ROOT && \
    ./bin/dreamland admin/travis/dreamland.xml 
}

travis_script() {
    run_build && run_smoke_test
}

set -e # stop on a non-zero exit code
set -x # display expanded commands

$1;

