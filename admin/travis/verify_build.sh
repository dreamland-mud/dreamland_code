#!/bin/bash

ROOT=$TRAVIS_BUILD_DIR

run_build() {
    mkdir -p objs && \
    make -f Makefile.git && \
    cd objs && \
    ../configure --prefix=$ROOT && \
    make -j 2 && make install 
}

run_smoke_test() {
    cd $ROOT && \
    mkdir -p share && \
    cd share && \
    git clone https://github.com/dreamland-mud/dreamland_world.git DL && \
    cd $ROOT && \
    ./bin/dreamland admin/travis/dreamland.xml 
}

travis_script() {
    run_build && run_smoke_test
}

travis_before_cache() {
    find . -name "lib*.so*" | xargs rm
    find . -name "lib*.cpp" | xargs rm
}

set -e # stop on a non-zero exit code
set -x # display expanded commands

$1;

