Вопросы, которые часто возникают при локальном тестировании изменений в коде.
Напомним, в локальной версии есть персонаж Кадм, с правами имплементора, а также доступом к Фене и OLC.
Можно пользоваться всем этим для тестирования.

### Как создать нового персонажа?
В базовую сборку добавлена феневая "корка" (база данных) с несколькими сценариями, в том числе и со сценарием Архивариуса, ответственным за создание новых персонажей. Для этого надо зайти на порт 9000, вместо 9001.

### Как добавить нового персонажа в мир без ребута
Пусть профайлы нового персонажа называются gosha.player и gosha.xml:
```bash
cd /home/dreamland/runtime
cp gosha.player var/db/oldstyle/backup
cp gosha.xml var/db/backup
```
Изнутри мира восстановим профайл и проверим, правильно ли создался персонаж:
```
recover gosha
finger gosha
```

### Как поменять профессию 
Установить профессию Кадму и выставить все умения на 100%:
```
eval profession=.Profession("warlock")
set skill kadm all 100
```
Установить профессию кому-то еще (из-под Кадма):
```
eval get_char_world("gosha").profession=.Profession("ninja")
set skill gosha all 100
```
Список всех профессий смотреть в: [dreamland_world/professions](https://github.com/dreamland-mud/dreamland_world/tree/master/professions) или /home/dreamland/runtime/share/DL/professions

### Как поменять родной город (hometown)
Аналогично, себе или кому-то:
```
eval hometown=.Hometown("solace")
eval get_char_world("gosha").hometown=.Hometown("midgaard_neutral")
```
Список всех городов смотреть в: [dreamland_world/hometowns](https://github.com/dreamland-mud/dreamland_world/tree/master/hometowns) или /home/dreamland/runtime/share/DL/hometowns

### Как поменять расу
```
eval race=.Race("satyr")
eval get_char_world("gosha").race=.Race("human")
```
Список всех рас: [dreamland_world/races](https://github.com/dreamland-mud/dreamland_world/tree/master/races) или /home/dreamland/runtime/share/DL/races, искать файлы с типом DefaultPCRace в корневой ноде XML.

### Как менять другие параметры мобов, персонажей и предметов
Есть божественная команда set, можно получить краткую справку по ее подкомандам:
```
set char
set obj
```
Помимо этого, многие поля можно изменить из Фени, однако не каждое поле будет доступно для записи.
Список полей и методов для разных сущностей можно быстро посмотреть, вызвав метод api() и отпечатав его игроку (ptc означает print to char):
```
eval ptc(api())                                # игрок (this, т.к. Кадм)
eval ptc(get_char_world("valkyr").api())       # моб, в данном случае Валькирия
eval ptc(.get_mob_index(3000).api())           # прототип моба
eval ptc(get_obj_world("barrel").api())        # предмет, в данном случае какой-то бочонок
eval ptc(.get_obj_index(3000).api())           # прототип предмета
eval ptc("some string".api())                  # строка
eval ptc(.api())                               # корневой объект
```
### Как заставить моба или персонажа выполнить команду
Можно воспользоваться божественной командой force или же Феней:
```
force valkyr smile
eval get_char_room("valkyr").interpret("smile")
```

### Как посмотреть списки всех доступных команд, умений, заклинаний
Божественные команды с краткой подсказкой:
```
wizhelp
```
Все игровые команды, умения и заклинания (для текущей профессии):
```
commands hints
skills all
spells all
```

### Как узнать русский синоним для английской команды и наоборот
Например, для команды look:
```
command show look
команды показ смотреть
```

### Как запустить глобальный квест, если в мире никого нет
Из-под Кадма запустите команду ```gq``` и прочтите синтаксис:
```
gquest start <id> [<min_level> <max_level>] [<time>] [<arg>] [<playerCnt>]
                   - запуск глобала:
                   - <id> имя глобала, список см. по gquest list
                   - <levels> указывают диапазн уровней для квестов типа gangsters
                   - <time> указывает длительность в минутах, по умолчанию 30
                   - <arg> указывает имя сценария, если они поддерживаются квестом
                   - <playerCnt> имитирует запуск квеста как будто онлайн такое кол-во игроков
```
Примеры:

* Запуск бандитов на 29 минут для уровней 5-10:
```
gq start gangsters 5 10 29
```

* Запуск радуги/грехов на 30 минут, как будто в мире 5 человек онлайн
```
gq start rainbow 30 rainbow 5
gq start rainbow 30 sins 5
```
* Запуск нашествия, как будто в мире 7 человек онлайн
```
gq start invasion 30 locust 7
```
* Остановка запущенного квеста:
```
gq stop gangsters
```
* Посмотреть, что сейчас запущено (квесты типа радуги выдают стартовое сообщение только через 1 минуту)
```
gq list
```