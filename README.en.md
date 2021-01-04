# DreamLand MUD developer instructions

<p align="center">
  <a href="https://github.com/dreamland-mud/dreamland_code/blob/master/README.md">Русский</a> |
  <span>English</span>
</p>

---

![DreamLand MUD version](https://img.shields.io/badge/DreamLand%20MUD-v4.0-brightgreen.svg)
[![License](https://img.shields.io/badge/License-GPL-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![Build Status](https://travis-ci.org/dreamland-mud/dreamland_code.svg?branch=master)](https://travis-ci.org/dreamland-mud/dreamland_code)
[![Discord chat](https://img.shields.io/discord/464761427710705664.svg?label=Discord%20chat&style=flat)](https://discord.gg/RPaz6ut)


Tested on clean Ubuntu 18.04/20.04 and gcc-9. You can either follow these instructions and create a local build environment,
or build a ready-to-use Docker container, as described in [dreamland_docker](https://github.com/dreamland-mud/dreamland_docker) README file.

## (optional) Install gcc-9 compiler
For Ubuntu 18.04 environments that have gcc-7 by default use the following steps to upgrade the compiler to version 9.3:

```bash
add-apt-repository ppa:ubuntu-toolchain-r/test
apt update
apt install gcc-9 g++-9
```

After that you can either add `export CXX=g++-9` to your bash profile, or make this a default compiler on the system:

```bash
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9
```

## Install build and dev tools
```bash
sudo apt-get update
sudo apt-get install -y git g++ gcc make automake libtool bison flex gdb telnet db-util
sudo apt-get install -y locales && locale-gen ru_RU.KOI8-R && update-locale
```

## Install dependency libraries
```bash
sudo apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev libfl-dev
```

## Download and build server code
```bash
git clone https://github.com/dreamland-mud/dreamland_code.git
cd dreamland_code
make -f Makefile.git
mkdir ../objs && cd ../objs
../dreamland_code/configure --prefix=/path/to/runtime
make -j 8 && make install
```

## Download configuration and areas
```bash
git clone https://github.com/dreamland-mud/dreamland_world.git
ln -s /path/to/dreamland_world /path/to/runtime/share/DL
```

## Start the game
```bash
cd /path/to/runtime
./bin/dreamland etc/dreamland.xml &
```

## View logs
Logs are available under /path/to/runtime/var/log.

## Accessing the game
```bash
telnet localhost 9127
```
user: Cadmus 
password: KadmKadm

This player is configured to have Russian skills/names/commands turned off in output, and is placed in room 31001 with English descriptions.

