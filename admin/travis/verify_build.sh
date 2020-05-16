#!/bin/bash

ROOT=$TRAVIS_BUILD_DIR

run_build() {
    du -s $HOME/.ccache
    mkdir -p objs
    make -f Makefile.git
    cd objs
    which gcc 
    export PATH=/usr/lib/ccache:$PATH
    which gcc
    ../configure --prefix=$ROOT 
    cat config.log
#    make -j 2 && make install 
}

run_smoke_test() {
    cd $ROOT && \
    git clone https://github.com/dreamland-mud/dreamland_world.git share/DL && \
    ./bin/dreamland admin/travis/dreamland.xml 
}

travis_script() {
#    run_build && run_smoke_test
    run_build
    du -s $HOME/.ccache
}

set -e # stop on a non-zero exit code
set -x # display expanded commands

$1;

