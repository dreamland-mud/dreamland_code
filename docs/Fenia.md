# Скриптовый язык Fenia

[[/images/fenia01.png]]

В мире Дримленд есть свой собственный скриптовый язык для описания сложных моделей поведения. Язык событийно-ориентированный, нетипизированный, с поддержкой многопоточности. Синтаксис его отдаленно напоминает JavaScript.

В Дримленд на этом языке написаны как простые сценарии (различные триггера на предметах и мобах), так и сложные: глобальные квесты, новые профессии, арийные квесты, и даже процедура создания нового персонажа.

Проще всего начать знакомство с феней, изучив собранные здесь уроки и примеры. 

## Уроки
* [Мяукающий кот](https://github.com/dreamland-mud/dreamland_code/wiki/Example-cat-meow)
* [Реакция на реплику с задержками](https://github.com/dreamland-mud/dreamland_code/wiki/Example-delayed-action)
* [Перемешивать выходы из комнаты ночью](https://github.com/dreamland-mud/dreamland_code/wiki/Example-randomize-exits)

## Документация
* [Феневое API](https://dreamland.rocks/feniaapi.html) - все поля и методы, доступные из скриптов
* [Триггера мобов и персонажей](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-char-triggers)
* [Триггера предметов](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-obj-triggers)
* [Триггера комнат](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-room-triggers)
* [[Формальное описание грамматики|Fenia Grammar]]

## Полезные статьи
* [Работа с редактором сценариев](https://github.com/dreamland-mud/dreamland_code/wiki/Fenia-editor)
* [[Беседа Филдса с Кинд|Fenia Log 01]] - старинные логи, где Филдс объясняет с азов, как пользоваться Феней. Может быть интересно как дополнение к урокам.
* [[Беседа Филдса с Анцифером|Fenia Log 02]] - еще один старинный лог, где Филдс объясняет, что такое потоки в Фене

## Примеры скриптов
Много рабочих примеров можно найти в каталоге `fenia.local` репозитория [dreamland_world](https://github.com/dreamland-mud/dreamland_world/tree/master/fenia.local). Некоторые из них снабжены комментариями. 

> **Осторожно**: Файлы могут быть в кодировке KOI8-R. Откройте их в сыром виде (кнопка `Raw`) и выберите нужную кодировку в браузере. 

* [Кубики из казино](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/(Kind)%20casino%20cubes%20v1.0.f%2B%2B) - простой пример триггера `onDrop`
* [Чертик из коробочки](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/(Kind)%20devil-in-the-box%20v1.0.f%2B%2B) - простой пример триггера `onOpen` и `onClose`
* [Школьный учитель и тренер](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/School%20adepts.f%2B%2B) - пример триггера на мобе, препятствующий выполнению команды
* [Учитель профессии татуировщик](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/tattoo%20practicer.f%2B%2B) - пример триггеров моба `onGreet` и `onSpeech`
* [Татуировочный нож](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/tattoo%20knife.f%2B%2B) - очень детальное, но почти линейное поведение ножа для татуажа, пример триггера `onUse` (`postUse`) и потоков
* [Поезд](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/Train%201.0.f%2B%2B) - поезд, ездящий по расписанию между несколькими станциями; это более сложный пример периодических действий с предметом, заданных через триггера `onSpec` и `onArea`
* [Еврей из Конторы Рестрингов в Мидгаарде](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/jew%20v1.5.f%2B%2B) - хороший пример сложного поведения моба, с ветвлениями диалогов и потоками
* [Архивариус: создание новых персонажей и вход старых](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/nanny%20v2.30.f%2B%2B) - еще один пример сложного диалогового поведения
* [Ария 'Машинные мечты'](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/area:%20machine%20dreams.f%2B%2B) - сбор комплекта брони, спецэффекты. Примеры триггера комнаты `onDescr`, реакции моба на социалы и эмоции (`postSocial`, `postEmote`)
* [Игры с огнем](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/(Cotton)%20fire%20v3.6.f%2B%2B) - реализованная с нуля поддержка костров и спичек. Примеры `onUse` для предметов, а также триггера для контейнеров.







