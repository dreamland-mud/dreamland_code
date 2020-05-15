#!/bin/bash

# Normally /home/travis/build/dreamland-mud/dreamland_code
ROOT=`pwd`

travis_before_script() {
    mkdir -p share && \
    cd share && \
    git clone https://github.com/dreamland-mud/dreamland_world.git DL
}

travis_script() {
    mkdir -p objs && \
    make -f Makefile.git && \
    cd objs && \
    ../configure --prefix=$ROOT && \
    make -j 2 && make install
} 

travis_after_success() {
    echo ">>> Starting and shutting down DreamLand..."
    ./bin/dreamland admin/travis/dreamland.xml
    echo ">>> All done."
}

