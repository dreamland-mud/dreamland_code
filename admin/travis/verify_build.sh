#!/bin/bash

set -e

set

# Normally /home/travis/build/dreamland-mud/dreamland_code
ROOT=`pwd`

echo ">>> Building DreamLand in $ROOT"

echo ">>> Installing dependency packages..."
sudo apt-get update
sudo apt-get install -y git g++ gcc make automake libtool bison flex telnet vim bzip2
sudo apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev db-util

echo ">>> Cloning dreamland_world..."
mkdir -p share && \
cd share && \
git clone https://github.com/dreamland-mud/dreamland_world.git DL

echo ">>> Configure and build the sources..."
cd $ROOT && \
mkdir -p objs && \
make -f Makefile.git && \
cd objs && \
../configure --prefix=$ROOT && \
make -j 2 && make install && \
echo ">>> Installation complete."

echo ">>> Running DreamLand..."
cd $ROOT && \
./bin/dreamland etc/dreamland.xml
echo ">>> Success!"


