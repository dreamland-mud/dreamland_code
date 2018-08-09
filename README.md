# DreamLand MUD developer instructions

Tested on clean Ubuntu 16.04, Ubuntu 14.04. You can either follow these instructions and create a local build environment,
or build a ready-to-use Docker container, as described in [dreamland_docker](https://github.com/dreamland-mud/dreamland_docker) README file.

## Install build and dev tools
```bash
sudo apt-get update
sudo apt-get install -y git g++ gcc make automake libtool bison flex gdb telnet vim
```

## Install dependency libraries
```bash
sudo apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev
```

## Download and build server code
```bash
git clone https://github.com/dreamland-mud/dreamland_code.git
cd dreamland_code
make -f Makefile.git
mkdir ../objs && cd ../objs
../dreamland_code/configure --path=/path/to/runtime
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
user: Kadm
password: KadmKadm

