#!/bin/bash

. $(dirname $(readlink -f $0))/paths

apt-get update
apt-get install -y git g++ gcc make automake libtool bison flex telnet vim bzip2 libfl-dev
apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev db-util
apt-get install -y locales && locale-gen ru_RU.KOI8-R && update-locale

mkdir -p $OBJS 
mkdir -p $SHARE
cd $SHARE
git clone https://github.com/dreamland-mud/dreamland_world.git DL
cd $SRCDIR

