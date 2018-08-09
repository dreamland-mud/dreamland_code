# DreamLand MUD, руководство разработчика

<p align="center">
  <span>Pусский</span> |
  <a href="https://github.com/dreamland-mud/dreamland_code/blob/master/README.en.md">English</a>
</p>

---

![DreamLand MUD version](https://img.shields.io/badge/DreamLand%20MUD-v4.0-brightgreen.svg)
[![License](https://img.shields.io/badge/License-GPL-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)

**Содержание**
* [Запуск локальной версии](#local)
   * [Подготовка окружения](#env)
   * [Сборка из исходников](#build)
   * [Установка dreamland_world](#areas)
   * [Запуск сервера](#run)
   * [Просмотр логов](#logs)
* [Работа с репозиторием](#git)
   * [Fork репозитория](#fork)
   * [Внесение изменений](#push)
   * [Pull requests](#pull)
* [Разработка](#dev)
   * [Работа с плагинами](#plugin)
  

## <a name="local">Запуск локальной версии</a>

Эта инструкция по сборке была проверена на Ubuntu 16.04 и Ubuntu 14.04. Если вам удалось собрать под чем-то еще, пожалуйста, обновите это руководство. Вы можете либо воспользоваться инструкцией и создать локальное окружение с нуля, либо собрать готовый к использованию Docker контейнер, как описано в Readme к проекту [dreamland_docker](https://github.com/dreamland-mud/dreamland_docker).

### <a name="env">Подготовка окружения</a>
Установите компилятор и сопутствующие программы, а также библиотеки, от которых зависит код дримленд:
```bash
sudo apt-get update
sudo apt-get install -y git g++ gcc make automake libtool bison flex gdb telnet vim
sudo apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev
```

### <a name="build">Сборка из исходников</a>
Склонируйте к себе либо главный репозиторий, либо свою собственную копию (fork) - о создании fork читайте ниже.

```bash
git clone https://github.com/dreamland-mud/dreamland_code.git
```
В каталоге с исходниками проинициализируйте конфигурационный скрипт и сборочные файлы, запустив
```bash
cd dreamland_code
make -f Makefile.git
```
В дальнейшем эту команду запускать не нужно, разве что если изменится configure.ac.
Приступаем к конфигурации и сборке. Для удобства все объектники будут в отдельном каталоге, чтобы не засорять исходники лишними файлами.
Инсталяция дримленд также будет в отдельном каталоге runtime, где на этапе конфигурации будет создано дерево каталогов и скопированы нужные файлы. 
```bash
mkdir ../objs && cd ../objs
../dreamland_code/configure --path=/path/to/runtime
make -j 8 && make install
```
### <a name="areas">Установка dreamland_world</a>
Склонируйте репозиторий dreamland_world, который содержит все конфигурационные файлы и некоторые зоны. 
Создайте на него ссылку из каталога runtime.
```bash
git clone https://github.com/dreamland-mud/dreamland_world.git
ln -s /path/to/dreamland_world /path/to/runtime/share/DL
```

Вот и всё, мир готов к запуску.

### <a name="run">Запуск сервера</a>

```bash
cd /path/to/runtime
./bin/dreamland etc/dreamland.xml &
```

Logs are available under /path/to/runtime/var/log.

```bash
telnet localhost 9127
```
user: Kadm
password: KadmKadm

