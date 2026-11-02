# Редактирование предметов

## Команда set obj
Через иммортальскую команду set obj можно менять числовые и строковые поля предметов, а также флаги.
Подробный синтаксис покажет команда, набранная без параметров:
```
set obj
```
Подробности работы с этой командой, а также значения value0-value4 и флагов описаны в 
```
help set obj
help 'PARAMETERS ПАРАМЕТРЫ'
help flags
```

Примеры:
1. Снять с предмета флаг take, например, чтобы превратить сумку в неподъемный сундук:
```
set obj bag wear -A
```
2. Добавить предмету гудение (флаг HUM):
```
set obj bag extra +B
```
3. Сделать мебель, на которой помещается два персонажа (поле value0):
```
set obj chair value0 2
```
4. Сделать мебель, на которой можно спать:
```
set obj chair value2 +K
```
5. Посмотреть, какие флаги установлены:
```
stat obj chair
```

## Феня

Аналогичных результатов можно достичь и Феней, меняя поля предмета value0, ... value4, а также extra_flags, wear_flags. Все флаги (extra, wear, флаги мебели, флаги контейнеров и так далее) хранятся в отдельных таблицах.
Посмотреть список всех таблиц можно, отпечатав себе вывод функции api():
```javascript
eval ptc(.tables.api())
```

Посмотреть список флагов отдельной таблицы можно, выведя ее api:
```javascript
eval ptc(.tables.furniture_flags.api())
```

Флаги хранятся в виде битовых масок, в которых можно убирать или устанавливать отдельные биты, а затем присваивать результат обратно полю предмета. Для работы с битовыми масками использутся такие методы корневого объекта:
```javascript
isset_bit: (mask, b) true если бит b установлен в mask (логическое 'и')
unset_bit: (mask, b) вернет mask со сброшенным битом b
set_bit: (mask, b) вернет mask с установленными битом b (логическое 'или')
```

Рассмотрим те же примеры:
1. Снять с предмета флаг take, например, чтобы превратить сумку в неподъемный сундук:
```javascript
eval bag=get_obj_carry("bag hero")
eval bag.wear_flags=.unset_bit(bag.wear_flags, .tables.wear_flags.take)
```
2. Добавить предмету гудение (флаг HUM):
```javascript
eval bag.extra_flags=.set_bit(bag.extra_flags, .tables.extra_flags.hum)
```
3. Сделать мебель, на которой помещается два персонажа (поле value0):
```javascript
eval chair=get_obj_here("chair")
eval chair.value0=2
```
4. Сделать мебель, на которой можно спать:
```javascript
eval chair.value2=.set_bit(chair.value2, .tables.furniture_flags.sleep_on)
```
5. Посмотреть, какие флаги установлены, можно через метод таблицы ```names```:
```javascript
eval ptc(.tables.extra_flags.names(bag.extra_flags))
eval ptc(.tables.furniture_flags.names(chair.value2))
```