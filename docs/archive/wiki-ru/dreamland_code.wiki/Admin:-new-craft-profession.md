# Как добавить новую доп. профессию

Весь код, описывающий дополнительные профессии, находится в плагине [craft](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/craft).


## Добавьте конфигурацию для новой профессии

Профайлы, описывающие дополнительные профессии, находятся в каталоге [craft-professions](https://github.com/dreamland-mud/dreamland_world/tree/master/craft-professions).

* В заголовке указано имя класса, реализующее данную профессию. По умолчанию это [CraftProfession](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/craft/subprofession.h). 
```xml
<CraftProfession type="CraftProfession">
```

* Внутри элемента ```help``` содержится справка по этой профессии. Аттрибут ```keyword``` содержит необязательные ключевые слова, по которым дополнительно должна находиться эта статья справки:
```xml
 <help level="-1" type="CraftProfessionHelp" keyword="TATTOO ТАТУИРОВКА">
```

* Опишите имена с падежами, в единственном (```rusName```) и множественном (```mltName```) числе.
* Элемент ```baseExp``` содержит базовый опыт на уровень.
* Элемент ```maxLevel``` содержит, сколько всего уровней планируется в этой профессии.

После этого в мире наберите:
```
plug reload craft
```
и новая профессия станет видна в справке, а также доступна через Феню (читать ниже).

## Проверьте, что профессия добавлена корректно

В корневом объекте есть метод ```.CraftProfession()```, создающий враппер вокруг доп. профессии и дающий доступ к ее полям и методам. Используйте метод ```api()``` для просмотра полного списка полей и методов:
```
eval prof=.CraftProfession("tattooist")
eval ptc(prof.nameMlt)
eval ptc(prof.api())
```

## Потестируйте набор опыта

Можно установить себе или тестовому персонажу 1й уровень новой профессии:
```
eval prof.setLevel(this, 1)
```
Затем набрать немного опыта:
```
eval prof.gainExp(this, 100)
```
И достичь следующего уровня мастерства в профессии, набрав ровно столько опыта, сколько оставалось до следующего уровня:
```
eval tnl=prof.getExpToLevel(ch)
eval prof.gainExp(this, tnl)
```

## Добавьте сценарий на Фене

Для того, чтобы новая профессия стала доступна для всех, надо придумать и реализовать способ, которым персонажи смогут достичь 1го уровня мастерства. В случае татуировщика за это отвечает [speech-прога](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/t1.f%2B%2B) на мобе, реагирующая на фразу "хочу стать татуировщиком". 

Другие примеры могут включать манипуляции с предметами или результат выполнения небольшого квеста.

***

# Как добавить новое умение (устарело, заменено skedit)

## Добавьте конфигурационный файл для умения

В Дримленд есть несколько типов умений: классовые, клановые, расовые, и т.д. Их конфигурационные файлы находятся в разных каталогах и плагинах. Для добавление нового скила-пустышки, который ничего не делает (его просто видно по команде ```slook```) достаточно добавить новый конфигурационный файл в соответствующий каталог и перегрузить соответствующий плагин (команда ```plugin reload```).

Типы умений:

* классовые: конфигурация в каталоге [generic-skills](https://github.com/dreamland-mud/dreamland_world/tree/master/generic-skills), код - каталог [groups](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/groups), плагин genericskill_loader
* клановые: конфигурация в каталоге [clan-skills](https://github.com/dreamland-mud/dreamland_world/tree/master/clan-skills), код - каталог [clan/impl](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/clan/impl), плагин clan_impl
* древние языки: конфигурация в каталоге [languages](https://github.com/dreamland-mud/dreamland_world/tree/master/languages), код - [languages/impl](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/languages/impl), плагин languages_impl
* доп. профессии: конфигурация в каталоге [craft-skills](https://github.com/dreamland-mud/dreamland_world/tree/master/craft-skills), код - [craft](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/craft), плагин craft
* умения колоды: конфигурация в каталоге [card-skills](https://github.com/dreamland-mud/dreamland_world/tree/master/card-skills), код - [cards](https://github.com/dreamland-mud/dreamland_code/tree/master/plug-ins/cards), плагин cards

Умения бывают нескольких видов, в зависимости от их использования:

* **Пустышки-аффекты**. Обычно хранятся там же, где и классовые умения. Не принадлежат ни к одной профессии, и существуют только для того, чтобы можно было повесить на персонажа аффект с таким именем. Секция в их профайле, отвечающая за аффект, может быть пустая (по умолчанию), а может описывать свои сообщения при спадании аффекта.

Пример таких умений: [коровье бешенство](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/cow%20lust.xml), [алтарь](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/altar.xml).

* **Пассивные умения**. Не имеют отдельной, связанной с ними команды или заклинания. В коде есть проверки на процент владения этим умением у персонажа, в результате этих проверок происходит или не происходит какое-то автоматическое действие. Их профайл содержит секцию ```classes```, описывающую с какого уровня это умения появляется у той или иной профессии, а также секцию ```raceBonuses```, описывающую расовые плюшки.

Пример таких умений: [увертка](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/dodge.xml), [быстрая поправка](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/fast%20healing.xml), [медитация](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/meditation.xml), [кинжал](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/dagger.xml).

* **Активные умения**. Имеют соответствующую им команду, часто одноименную с умением, чья конфигурация (имена, позиция, флаги) описывается в отдельной секции.

Примеры: [облако пыли](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/blindness%20dust.xml) - команда ```облакопыли```, [подкрадывание](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/sneak.xml) - команда ```красться``` и так далее.

* **Заклинания и молитвы**. Колдуются с помощью команды ```колдовать```. Имеют отдельную секцию конфигурации, описывающую тип и цели заклинания.

Примеры: [благословение](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/bless.xml),  [адамантитовый голем](https://github.com/dreamland-mud/dreamland_world/blob/master/generic-skills/adamantite%20golem.xml)

Новые умения можно добавлять полностью по аналогии с существующими, заменяя имена, справку и флаги на нужные. 



