# DreamLand MUD

[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)
![DreamLand MUD version](https://img.shields.io/badge/DreamLand%20MUD-v4.0-brightgreen.svg)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)
[![Discord chat](https://img.shields.io/discord/969942531980808243.svg?label=Discord%20chat&style=flat)](https://discord.gg/fVtaeePyey)

DreamLand is a text multiplayer game (MUD) in the DikuMUD / ROM / Anatolia family, running since the 1990s. This repository is the server: a C++17 core library, about 50 plugins, and the Fenia scripting interpreter. Game data lives in [dreamland_world](https://github.com/dreamland-mud/dreamland_world), public Fenia scripts in [dreamland_fenia_public](https://github.com/dreamland-mud/dreamland_fenia_public), the web client in [mudjs](https://github.com/dreamland-mud/mudjs). You can play at https://dreamland.rocks/play or by telnet to dreamland.rocks:9000.

The README below is a quick start. The wiki has the rest:

* [Local-Setup](https://github.com/dreamland-mud/dreamland_code/wiki/Local-Setup): environments, runtime layout, ports, logging in, what to rebuild after a change
* [Local-Testing](https://github.com/dreamland-mud/dreamland_code/wiki/Local-Testing): how to check your change
* [Troubleshooting](https://github.com/dreamland-mud/dreamland_code/wiki/Troubleshooting): if the server does not start
* [Contribute](https://github.com/dreamland-mud/dreamland_code/wiki/Contribute): how to send a change
* [Agent-Index](https://github.com/dreamland-mud/dreamland_code/wiki/Agent-Index): wiki map for tools and AI agents

## Requirements

Ubuntu 18.04 or 20.04 (real machine, VM, WSL 2 or a container) and gcc 9 or newer. The build uses gcc-only flags, so other compilers will not work. macOS is not supported, use Docker or a Linux VM.

This repository has `Dockerfile.localbuild` (Ubuntu 20.04 build container), `Dockerfile.localrun` (runtime container) and `Dockerfile.gitpod` with `.gitpod.yml` for Gitpod.

### Build tools and libraries

```bash
sudo apt-get update
sudo apt-get install -y git g++ gcc make automake libtool bison flex gdb telnet db-util bzip2 perl
sudo apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev libfl-dev
sudo apt-get install -y locales && sudo locale-gen uk_UA && sudo update-locale
```

The `uk_UA` locale is used for Cyrillic sorting. Without it the server starts but sorts Russian text wrongly.

### Optional: gcc 9 on Ubuntu 18.04

Ubuntu 18.04 ships gcc 7, which is too old (no `<filesystem>`). Do this after the packages above, so that the `g++` and `gcc` packages do not replace the alternatives links. To get gcc 9:

```bash
sudo apt-get install -y software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install -y gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9
```

Check with `g++ --version` before you run `configure`. If it still shows 7, run `export CC=gcc-9 CXX=g++-9` first.

## Build and install

Sources, build directory and runtime directory are three separate folders. Replace `/path/to/runtime` with your install location. The build directory `../objs` is also written as `/path/to/objs` below.

```bash
git clone https://github.com/dreamland-mud/dreamland_code.git
cd dreamland_code
make -f Makefile.git
mkdir ../objs && cd ../objs
../dreamland_code/configure --prefix=/path/to/runtime
make -j 4 && make install
```

`make -f Makefile.git` generates the `configure` script. Run it again only when `configure.ac`, a `Makefile.am`, a `Makefile.inc` or an `.m4` file changes. `configure` also creates the runtime tree under the prefix: `bin`, `lib`, `libexec`, `etc/dreamland.xml`, the `var` folders, the Fenia database and two default administrator characters. It does not overwrite files that already exist. The first full build takes a while.

## Download configuration and areas

```bash
cd /path/to/runtime
git clone https://github.com/dreamland-mud/dreamland_world.git share/DL
git clone https://github.com/dreamland-mud/dreamland_fenia_public.git share/DL/fenia/public
```

`dreamland_world` holds the game configuration and a small set of sample zones in `areas.local`. The stock `etc/dreamland.xml` loads only those zones. See [Repositories](https://github.com/dreamland-mud/dreamland_code/wiki/Repositories) for how the pieces fit.

## Start the game

```bash
cd /path/to/runtime
./bin/dreamland etc/dreamland.xml &
```

With the stock `etc/dreamland.xml` there is no `logPattern` setting, so the log goes to standard output. To write log files instead, stop the server (see below) and add this line to `etc/dreamland.xml`. The server writes its in-memory copy of that file back every game tick and on shutdown, so an edit made while it runs is lost. Changes apply at the next start.

```xml
<logPattern>var/log/%Y%m%d-%H%M%S.log</logPattern>
```

Stop the server with SIGINT or SIGTERM (Ctrl-C in the foreground, or `kill %1` for the background job started above). The server rewrites `etc/dreamland.xml` while it runs, so expect that file to change in your tree. Always stop the server before you edit it.

## Log in

| Port | Use |
|---|---|
| 9000 | Telnet with the normal login and character creation dialog |
| 9001 | Telnet backdoor, login on one line |
| 9127 | Telnet backdoor, local connections only |
| 1234 | WebSocket, the protocol [mudjs](https://github.com/dreamland-mud/mudjs) speaks. The stock mudjs build connects only to dreamland.rocks, so use telnet locally (see [Client-Web-Development](https://github.com/dreamland-mud/dreamland_code/wiki/Client-Web-Development)) |

The backdoor ports expect one line with a codepage number from 1 to 7 first, then name and password. Number 7 is UTF-8:

```text
7 Kadm KadmKadm
```

Two characters exist after the first install, both level 107 with password `KadmKadm`. `Kadm` uses Russian commands and output. `Cadmus` uses English. Use them for local development only and never on a server other people can reach.

```bash
telnet localhost 9000
```

If the server runs in a VM or container, use its address instead of `localhost`.

Fenia scripts from `dreamland_fenia_public` load with `cs load public/<path>` from inside the game. For that to work, stop the server, set `feniaScriptDir` in `etc/dreamland.xml` to `share/DL/fenia`, and start it again. The stock value is `share/DL/fenia.local`. See [Fenia-Scripts](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-Scripts).

## Rebuild after a change

A change in one plugin:

```bash
cd /path/to/objs/plug-ins/yourplugin
make -j 4 && make install
```

Then, inside the game:

```text
plugin reload changed
```

A change to `.cpp` files in `src/` only needs a rebuild of `src` and a server restart:

```bash
cd /path/to/objs/src
make -j 4 && make install
```

A change to a header in `src/`, or to a header shared by plugins, needs a full build and a restart:

```bash
cd /path/to/objs
make -j 4 && make install
```

Do not use `plugin reload` after a core header change. Restart the server instead. Details are on [Local-Setup](https://github.com/dreamland-mud/dreamland_code/wiki/Local-Setup).

## Contributing

Fork the repository, make your change on a branch, and open a pull request against `master`. See [Contribute](https://github.com/dreamland-mud/dreamland_code/wiki/Contribute) for the workflow and code style. Questions go to the [Discord](https://discord.gg/fVtaeePyey).

## License

GPLv3. See `LICENSE`.
