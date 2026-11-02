# Скриптовый язык Fenia

[[/images/fenia01.png]]

В Мире Мечты есть свой собственный скриптовый язык для описания сложных моделей поведения. Язык событийно-ориентированный, нетипизированный, с поддержкой многопоточности. Синтаксис его отдаленно напоминает JavaScript.

На этом языке у нас написаны как простые сценарии (различные триггера на предметах и мобах), так и сложные: большинство умений, глобальные квесты, новые профессии, арийные квесты, и даже процедура создания нового персонажа.

## Документация
* [[Много примеров и образцов кода|Fenia:-Examples]]
* [[Логи обучающих бесед о Фене|Fenia:-Tutorials]]
* [Феневое API](https://dreamland.rocks/feniaapi.html) — все поля и методы, доступные из скриптов
* [[Триггеры всех событий|Fenia:-Triggers]]
* [[Работа с умениями и навыками|Fenia:-Skills-and-spells]]
* [[Формальное описание грамматики|Fenia:-Grammar]]

***

# Работа с редактором скриптовых сценариев

Для работы со сценариями используется команда ```codesource``` (```cs```). Подробный синтаксис можно посмотреть, набрав ее изнутри мира. Исторически было много разных способов добавить сценарий в мир, и все они поддерживаются, но на сегодня самой удобной является подкоманда ```cs web```. Для ее использования необходимо присоединиться через веб-клиент: 

* [версия клиента для стройплощадки](https://dreamland.rocks/play/build.html)
* [инструкция по использованию веб-клиента для локального сервера](https://github.com/dreamland-mud/mudjs/blob/dreamland/README.md)

Сперва посмотрим, какие сценарии уже есть в мире, командой ```cs list```. Много рабочих примеров можно найти в каталоге `fenia.local` репозитория [dreamland_world](https://github.com/dreamland-mud/dreamland_world/tree/master/fenia.local). Некоторые из них снабжены комментариями. 

> **Осторожно**: Файлы могут быть в кодировке KOI8-R. Откройте их в сыром виде (кнопка `Raw`) и выберите нужную кодировку в браузере. 

* [Кубики из казино](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/(Kind)%20casino%20cubes%20v1.0.f%2B%2B) - простой пример триггера `onDrop`
* [Чертик из коробочки](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/(Kind)%20devil-in-the-box%20v1.0.f%2B%2B) - простой пример триггера `onOpen` и `onClose`
* [Школьный учитель и тренер](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/School%20adepts.f%2B%2B) - пример триггера на мобе, препятствующий выполнению команды
* [Учитель профессии татуировщик](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/tattoo%20practicer.f%2B%2B) - пример триггеров моба `onGreet` и `onSpeech`
* [Татуировочный нож](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/tattoo%20knife.f%2B%2B) - очень детальное, но почти линейное поведение ножа для татуажа, пример триггера `onUse` (`postUse`) и потоков
* [Поезд](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/Train%201.0.f%2B%2B) - поезд, ездящий по расписанию между несколькими станциями; это более сложный пример периодических действий с предметом, заданных через триггера `onSpec` и `onArea`
* [Еврей из Конторы Рестрингов в Мидгаарде](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/jew%20v1.5.f%2B%2B) - хороший пример сложного поведения моба, с ветвлениями диалогов и потоками
* [Архивариус: создание новых персонажей и вход старых](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/nanny.f%2B%2B) - еще один пример сложного диалогового поведения
* [Ария 'Машинные мечты'](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/area:%20machine%20dreams.f%2B%2B) - сбор комплекта брони, спецэффекты. Примеры триггера комнаты `onDescr`, реакции моба на социалы и эмоции (`postSocial`, `postEmote`)
* [Игры с огнем](https://github.com/dreamland-mud/dreamland_world/blob/master/fenia.local/(Cotton)%20fire%20v3.6.f%2B%2B) - реализованная с нуля поддержка костров и спичек. Примеры `onUse` для предметов, а также триггера для контейнеров.

![cs list output](https://dreamland.rocks/downloads/csweb02.png)

Начнем добавлять новый сценарий. 
```
cs web
```
![cs web empty](https://dreamland.rocks/downloads/csweb01.png)

Открывшийся терминал подсвечивает синтаксис (разные элементы языка разными цветами), сам закрывает фигурные скобки и делает отступы. Самые грубые ошибки подсветятся красным. 

Составим новый сценарий для такой задачи:
_адепт Дваркина в MUD-школе тренирует только до 20го уровня, хотя заходить к нему могут все. Он должен отказывать в услуге по достижению персонажем 20го уровня. Триггер onCantTrain вызывается, когда набрали команду ```тренировать``` (```train```)._

Вобьем что-нибудь осмысленное в поле Subject, а в основном окне редактора введем такой сценарий:

```javascript
.apply(function() {
	// Dwarking adept: no training after 20 level.
	.get_mob_index(3718).onCantTrain = function(adept, client) {
		if (client.getModifyLevel() >= 20) {
			adept.recho("%^C1 сокрушенно цокает языком.", adept);
			adept.say("Ты уже взросл%1$Gое|ый|ая, %1$C1, поищи себе другого тренера.", client);
			return true;
		}
		return false;
	};
	
	// NoFate adept: no practicing after 20 level.
	.get_mob_index(3719).onCantTeach = function(adept, client, skillName) {
		if (client.getModifyLevel() >= 20) {
			var skill;
			skill = .Skill(skillName);
			
			adept.say("Извини, %1$^C1, но теперь тебе придется искать себе другого учителя.", client);
			adept.say("Учителя для этого умения можно найти в справке по умению.", client);
			return true;
		}
		return false;
	};
})
```

По нажатию на кнопку Run этот сценарий попытается выполниться (кнопка Run - аналог подкоманды `cs post`). Если сценарий не содержит ошибок, то соответствующие тригера onTrain присвоятся прототипам мобов-адептов, и сценарий станет видно в списке.

Метод `.apply(...)` принимает в параметры функцию, которая вызовется при вызове `.apply()`. А сам `.apply()`, в свою очередь, вызовется по нажатию на кнопку Run. Таким образом удается удобно выполнить все необходимые присваивания.

Cписок активных триггеров на прототипе моба можно отпечатать себе так:
```
eval ptc(.get_mob_index(3718).rtapi())

Runtime fields:
onCantTrain
```

Наконец, по команде ```cs web номер``` можно начать редактировать новый сценарий, при этом тема и тело сценария будет скопировано из существующего сценария с указанным номером. После выполнения инициализирующей функции, новый сценарий займет место предыдущего. 

```
cs list
```
![cs list again](https://dreamland.rocks/downloads/csweb04.png)


