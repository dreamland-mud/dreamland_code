#!/bin/bash

ROOT=/workspace
SRCDIR=$ROOT/dreamland_code
RUNTIME=$ROOT/runtime
OBJS=$ROOT/objs
SHARE=$RUNTIME/share

apt-get update
apt-get install -y git g++ gcc make automake libtool bison flex telnet vim bzip2
apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev db-util

mkdir $OBJS && \
cd $SRCDIR && \
make -f Makefile.git && \
cd $OBJS && \
../dreamland_code/configure --prefix=$RUNTIME && \
make -j 2 && make install && \
cd $SHARE && \
git clone https://github.com/dreamland-mud/dreamland_world.git && \
mv dreamland_world DL && \
cd $SRCDIR
