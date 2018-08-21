# DreamLand MUD, руководство разработчика

<p align="center">
  <span>Pусский</span> |
  <a href="https://github.com/dreamland-mud/dreamland_code/blob/master/README.en.md">English</a>
</p>

---

![DreamLand MUD version](https://img.shields.io/badge/DreamLand%20MUD-v4.0-brightgreen.svg)
[![License](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0.html)

**Содержание**
* [Запуск локальной версии](#local)
   * [Подготовка окружения](#env)
   * [Сборка из исходников](#build)
   * [Установка dreamland_world](#areas)
   * [Запуск сервера](#run)
   * [Просмотр логов](#logs)
   * [Вход в мир](#telnet)
* [Работа с репозиторием](#git)
   * [Fork репозитория](#fork)
   * [Внесение изменений](#push)
   * [Pull requests](#pull)
* [Разработка](#dev)
   * [Пересборка 'ядра'](#core)
   * [Пересборка плагинов](#plugin)

## <a name="local">Запуск локальной версии</a>

Эта инструкция по сборке была проверена на Ubuntu 16.04 и Ubuntu 14.04. Дримленд гарантированно собирается и работает под версиями компилятора:
* gcc 5.4
* gcc 4.8

Гарантированно не собирается под 
* gcc 6.3

Если вам удалось собрать под чем-то еще, пожалуйста, обновите это руководство. 

Вы можете либо воспользоваться инструкцией и создать локальное окружение с нуля, либо собрать готовый к использованию Docker контейнер, как описано в Readme к проекту [dreamland_docker](https://github.com/dreamland-mud/dreamland_docker).

### <a name="env">Подготовка окружения</a>
Установите компилятор и сопутствующие программы, а также библиотеки, от которых зависит код дримленд:
```bash
sudo apt-get update
sudo apt-get install -y git g++ gcc make automake libtool bison flex gdb telnet vim
sudo apt-get install -y libcrypto++-dev libjsoncpp-dev libdb5.3 libdb5.3-dev libdb5.3++ libdb5.3++-dev zlib1g zlib1g-dev libssl-dev
```

### <a name="build">Сборка из исходников</a>
Склонируйте к себе либо главный репозиторий, либо свою собственную копию (fork) - о создании fork читайте ниже.
Предположим, что исходники будут лежать в `/home/dreamland/dreamland_code`, тогда:
```bash
mkdir /home/dreamland && cd /home/dreamland
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
В этом руководстве предполагается, что объектники лежат в `/home/dreamland/objs`, а инсталляция - в `/home/dreamland/runtime`. Измените пути в примерах согласно своей конфигурации.

```bash
mkdir /home/dreamland/objs && cd /home/dreamland/objs
/home/dreamland/dreamland_code/configure --prefix=/home/dreamland/runtime
make -j 8 && make install
```
### <a name="areas">Установка dreamland_world</a>
Склонируйте репозиторий dreamland_world, который содержит все конфигурационные файлы и некоторые зоны. 
Создайте на него ссылку из каталога runtime.
```bash
cd /home/dreamland
git clone https://github.com/dreamland-mud/dreamland_world.git
ln -s /home/dreamland/dreamland_world /home/dreamland/runtime/share/DL
```

Вот и всё, мир готов к запуску.

### <a name="run">Запуск сервера</a>

```bash
cd  /home/dreamland/runtime
./bin/dreamland etc/dreamland.xml &
```
### <a name="logs">Просмотр логов</a>

Логи попадают в подкаталог `var/log` в каталоге runtime. Формат файла логов задается в `etc/dreamland.xml`, по умолчанию имя файла - это дата и время запуска. 
```xml
<logPattern>var/log/%Y%m%d-%H%M%S.log</logPattern>
```
Удалив эту строку из dreamland.xml, можно добиться вывода логов в stdout.

### <a name="telnet">Вход в мир</a>

Мир доступен локально на порту 9127, например:
```bash
telnet localhost 9127
```
Там есть только один персонаж, наделенный всеми полномочиями: Kadm, пароль KadmKadm. 
При входе через порт 9127 укажите кодировку, логин и пароль, например: 0 Kadm KadmKadm

---
## <a name="git">Работа с репозиторием</a>

### <a name="fork">Fork репозитория</a>

Создайте свою собственную копию (fork) репозитория, нажав на кнопку Fork вверху [страницы](https://github.com/dreamland-mud/dreamland_code):
![fork example](https://dreamland.rocks/img/git01.png)

Ваша копия будет иметь путь https://github.com/yourname/dreamland_code:
![fork example](https://dreamland.rocks/img/git02.png)

Cклонируйте исходники к себе на машину, используя URL из Clone or download:
![clone example](https://dreamland.rocks/img/git0211.png)
например
```bash
git clone https://github.com/yourname/dreamland_code
```

### <a name="push">Внесение изменений</a>
Теперь вы можете вносить какие угодно изменения в свой fork, никак не влияя на основной репозиторий. Когда какая-то функциональность будет готова к вливанию обратно в основной репозиторий, нужно будет создать запрос (pull request), об этом ниже.

Кратко опишем команды, которые понадобятся для внесения изменений в свой fork. Все это стандартные команды git, о которых можно прочитать во многих руководствах.
1. Проверить, какие файлы изменились или добавились в локальной версии:
```bash
git status
```
Просмотреть изменения подробно:
```bash
git diff
```
2. Добавить все измененные файлы в будущий commit:
```bash
git add .
```
Добавить файлы выборочно:
```bash
git add path/to/file
```
3. Создать commit и описать изменение. Описания рекомендуется делать понятные для тех, кто будет читать их через полгода.
```bash
git commit -m "Guys, I did a thing!"
```
Если нужно запустить встроенный редактор для создания описания (commit log):
```bash
git commit
```
4. Выпихнуть изменения на github:
```bash
git push
```

### <a name="pull">Pull requests</a>
Настало время поделиться со всеми тем, над чем вы корпели так долго. 
На странице вашего репозитория будет описано, на сколько коммитов вы опережаете родительский репозиторий, и появится кнопка для создания запроса New pull request:
![pull example](https://dreamland.rocks/img/git04.png)

Нажав на нее, можно будет просмотреть отличия между двумя ветками. Если между вашей и родительской версией нету конфликтов, вы увидите "Able to merge". Можно создавать pull request, нажав на кнопку Create:
![pull example](https://dreamland.rocks/img/git05.png)

---

## <a name="dev">Разработка</a>

Несколько замечаний, которые могут облегчить жизнь при разработке.

### <a name="core">Пересборка 'ядра'</a>
Если вы внесли изменения в каталог src:
* если поменялась только реализация (файлы с расширением .cpp), достаточно пересобрать только каталог src:
```bash
cd /home/dreamland/objs/src
make -j 4 && make install
```
* если ваше изменение также повлияет и на плагины (например, поменялся заголовочный файл) - то нужно пересобрать вообще всё.  
```bash
cd /home/dreamland/objs
make -j 4 && make install
```
Затем надо перезапустить dreamland (см. выше про запуск).

### <a name="plugin">Пересборка плагинов</a>
Пересоберите все измененные плагины:
```bash
cd /home/dreamland/objs/plug-ins/yourplugin
make -j 4 && make install
```
Перегрузите все измененные плагины изнутри мира, набрав:
```
plug reload changed
```



