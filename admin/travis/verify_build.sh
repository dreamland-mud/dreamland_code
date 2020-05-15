#!/bin/bash

ROOT=$TRAVIS_BUILD_DIR

run_build() {
    which gcc
    which g++
    which ccache
    echo $PATH

    mkdir -p objs && \
    make -f Makefile.git && \
    cd objs && \
    ../configure --prefix=$ROOT && \
    make -j 2 && make install 
}

run_smoke_test() {
    cd $ROOT && \
    git clone https://github.com/dreamland-mud/dreamland_world.git share/DL && \
    ./bin/dreamland admin/travis/dreamland.xml 
}

travis_script() {
    run_build && run_smoke_test
}

set -e # stop on a non-zero exit code
set -x # display expanded commands

$1;

