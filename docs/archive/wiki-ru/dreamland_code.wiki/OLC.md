# Поваренная книга, или справочник по OLC
(оригинал этой книги находится в мире в библиотеке на Улице Вязов)

## Введение

OLC -- это плагин, реализующий набор команд, предназначенных для редактирования арий. В различных состояниях набор команд отличается: например, состояние редактирования комнаты, редактирование моба или предмета. Обычно командный интерпретатор находится в начальном состоянии. В этом состоянии доступны все стандартные команды.

При переходе из одного состояния в другое, предыдущее запоминается. В каждом из состояний предусмотрена возможность вернуться к предыдущему.
К примеру, не завершив редактирования комнаты, можно начать редактировать объект. После завершения завершения редактирования объекта, продолжится редактирование комнаты.

## alist: список зон
Посмотреть полный список зон и их диапазоны внумов.

## aedit: редактор зон
Синтаксис:
* edit area — редактировать текущую зону
* edit area <номер> — редактировать зону с номером <номер>. Номер существующий эрии можно узнать с помощью команд alist. (см. alist)
* edit area create — создает новую эрию со следующим свободным номером и переводит редактор в состояние редактирования новосозданной эрии.

Следующие команды доступны только из состояния редактирования эрии:
* age <число> — установить возраст эрии.
* builder <чар1 чар2 ...> — установить список редакторов. (*)
* create — синоним edit area create.
* filename <строка> — установить имя файла эрии.
* name <строка> — установить имя эрии.
* reset — обновить эрию (repop).
* security <число> — установить необходимый уровень приврлегий. (*)
* show — показать текущую ин-ю о редактируемой эрии.
* vnum <число> <число> — установить диапазон vnum'ов.
* lvnum <число> — установить нижнюю границу диапазона vnum'ов.
* uvnum <число> — установить верхнюю границу диапазона vnum'ов.
* levels <число> <число> — установить рекомендуемый диапазон левелов.
* credits <строка> — установить строку описания.
* resetmsg <строка> — установить reset message.
* flag <флаги> — установить флаги (см. olchelp)
(*): не поддерживается из-за совместимости со старым форматом эрий.

## asave: сохранение зон
Команда для сохранения эрий на диск.
* asave <внум> — сохранить одну эрию с номером <внум>. номер можно узнать из alist
* asave list — сохранить только список эрий
* asave area — сохранить эрию в которой находится чар
* asave changed — сохранить измененные эрии
* asave world — сохранить все эрии

## redit: редактор комнат
* edit room — переводит редактор в состояние редактирования комнаты в которой находится чар.
* edit room <внум> — переводит редактор в состояние редактирования комнаты с номером <vnum>.
* edit room create <внум> — создает новую комнату с номером <vnum> и переводит редактор в состояние редактирования новосозданной комнаты.
* edit room create next	— создает новую комнату со следующим свободным внумом
* edit room reset — обновляет редактируемую комнату.
* edit room show <внум>	— показывает информацию о комнате <vnum>

Следующие команды доступны только из состояния редактирования комнаты:
* create <внум> — синоним edit room create <внум>
* create next — синоним edit room create next
* desc — устанавливает описание, переводит редактор в состояние редактирования строки (см. string)
* ed — см. https://github.com/dreamland-mud/dreamland_code/wiki/OLC:-ED-editor-(obsolete)
* format — форматировать описание
* name <строка> — установить имя комнаты
* show или '' (пустая строка) — показать инф-ю о редактируемой комнате
* heal <число> — скорость восстановления hp. 100 - нормальная скорость
* mana <число> — скорость восстановления mana. 100 - нормальная скорость
* north, south, ... — см. exits
* owner <строка> — установить владельца
* clan <строка> — установить клан
* pstore <внум> — установить vnum хранилища животных (если не следующий)
* eexit ... — см. eexits
* olist <all/тип/имя> — список объектов в этой эрии ( all - всех, тип - с заданым типом (см. olchelp), имя - с заданым именем)
* rlist — список комнат в этой эрии
* room_flags — см. olchelp
* sector_type — см. olchelp
* mreset <внум> <макс> <мир> — добавить ресет моба в редактируемой комнате. <внум> - номер моба, <макс> - максимум мобов в комнате, <мир> - максимум мобов в мире. (*)
* oreset <внум> — добавить ресет объекта в редактируемой комнате. <vnum> - номер объекта. объект будет ресетиться на полу. (*)
* oreset <внум> <строка> - добавить ресет объекта в редактируемой комнате. <vnum> - номер объекта. <строка> - имя объекта, в котором должен ресетиться добавляемы. (*)
* oreset <внум> <строка> <wear-loc> — добавить ресет объекта в редактируемой комнате. объект с номером <vnum> будет надет на моба с именем <строка> на wear location <wear-loc>. (*) (см. olchelp)

## north: редактирование выходов
Команды east, north, south, west, up, down предназначены для редактирования стандартных выходов из комнаты. Доступны только из состояния редактирования комнаты. Исключение составляет первый вариант команды, который дублируется в стандартном наборе команд.
* north — переместить чара в направлении. Эта команда не изменяет состояние редактора. Вместо этого редактирование переходит к комнате в которую попал чар.
* north <exit_flags> — изменить флаги выхода (см. olchelp)
* north ? — синоним olchelp exit
* north delete — удалить двусторонний проход. если проход односторонний, или выход с другой стороны ведет в другую комнату, удаляется только односотронний проход.
* north link <vnum> — создать двусторонний проход с существующей комнатой <vnum>.
* north dig <vnum> — создать двусторонний проход в новую комнату. созданая комната будет иметь номер <vnum>.
* north dig next — создать двусторонний проход в новую комнату. созданая комната будет иметь следующий свободный номер.
* north room <vnum> — создать односторонний проход в существующую комнату <vnum>.
* north key <vnum> — установить ключ (для двери)
* north name <строка> — установить имя выхода (для open door, lock door)
* north descr — установить описание выхода (для look north). переводит редактор в состояние редактирования строки. (см. string)

### eexit: редактор экстра-выходов
Команда для редактирования экстра-выходов. Доступна исключительно из состояния редактирования комнаты.
eexit delete <слово> — удаляет экстравыход с ключевым <словом>.
eexit set <слово> — добавляет экстравыход с ключевыми <словами>. Эта команда переводит редактор в состояние редактирования экстравыхода.
eexit <слово> — редактирует экстравыход <слово>. Эта команда переводит редактор в состояние редактирования экстравыхода.

Если название экстравыхода (<слово>) должно состоять из нескольких слов, его необходимо брать в кавычки. В состоянии редактирования экстравыходов доступны следующие команды:
show / '' (пустая строка) — показать текущую инф-ю о редактируемом экстравыходе.
desc — редактировать описание. (для look keyword). переводит редактор в состояние редактирования строки. (см. string)
rdesc — редактировать описание комнаты. (добавляется к описанию комнаты при look, если чар может видеть этот экстравыход). переводит редактор в состояние редактирования строки. (см. string)
name <строка> — изменить ключевое слово.
key <vnum> — установить ключ.
target <vnum> — установить комнату назначения.
from <число1> <число2> <строка> — сообщение тем, кто остался в комнате, из которой ушел чар. (*)
to <число1> <число2> <строка> — сообщение тем, кто находился в комнате, в которую пришел чар. (*)
<eexit_flags> — см. olchelp
<size> — установить максимальный размер чара, способного пройти в этот экстравыход. (см. olchelp)
(*): Сообщение для from формируется таким образом: ru rt short.
Сообщение для to формируется таким образом: rp rt short. здесь:
* ru — задается <числом1> из команды from,
* rp - <числом1> из команды to,
* rt - <числом2> одинакого для from и to,
* short - <строка> одинакого для from и to.

<table cellspacing="0" id="table3" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" colspan="2" valign="top" class="twikiTableCol0 twikiFirstCol"> &lt;ru&gt; </td>
			<td bgcolor="#ffffff" align="center" colspan="2" valign="top" class="twikiTableCol2"> &lt;rp&gt; </td>
			<td bgcolor="#ffffff" align="center" colspan="2" valign="top" class="twikiTableCol4"> &lt;rt&gt; </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 0 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> ушел </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol2"> 0 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol3"> пришел </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol4"> 0 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol5 twikiLastCol"> в </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> 1 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1"> взобрался </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol2"> 1 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol3"> забрался </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol4"> 1 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol5 twikiLastCol"> на </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 2 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> запрыгнул </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol2"> 2 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol3"> запрыгнул </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol4"> 2 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol5 twikiLastCol"> сквозь </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> 3 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1"> бросился </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol2"> 3 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol3"> упал </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol4"> 3 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol5 twikiLastCol"> между </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 4 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> нырнул </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol2"> 4 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol3"> донырнул </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol4"> 4 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol5 twikiLastCol"> над </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> 5 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1"> уплыл </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol2"> 5 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol3"> приплыл </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol4"> 5 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol5 twikiLastCol"> через </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 6 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> всплыл </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol2"> 6 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol3"> всплыл </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol4"> 6 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol5 twikiLastCol"> под </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> 7 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1"> протиснулся </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol2"> 7 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol3"> протиснулся </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol4"> 7 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol5 twikiLastCol"> с </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 8 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> улетел </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol2"> 8 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol3"> прилетел </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol4"> 8 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol5 twikiLastCol"> из </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> 9 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1"> спрыгнул </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol2"> 9 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol3"> спрыгнул </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol4"> 9 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol5 twikiLastCol"> со </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 10 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> слез </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol2"> 10 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol3"> слез </td>
			<td bgcolor="#edf4f9" align="right" valign="top" class="twikiTableCol4"> 10 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol5 twikiLastCol"> из под </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> 11 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLast"> спустился </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol2 twikiLast"> 11 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol3 twikiLast"> спустился </td>
			<td bgcolor="#ffffff" align="right" valign="top" class="twikiTableCol4 twikiLast"> 11 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol5 twikiLastCol twikiLast"> по </td>
		</tr>
	</tbody></table>

## medit: редактор мобов
Команда для редактирования мобов.
* edit mobile <vnum> — редактировать моба с номером <vnum>. Переводит редактор в состояние редактирования мобов.
* edit mobile create <vnum> — создать нового моба с номером <vnum>. Переводит редактор в состояние редактирования мобов.
* edit mobile show <vnum> — показать информацию о мобе с номером <vnum>.
* edit mobile load <vnum> — загрузить монстра с номером <vnum>. Монстр появится рядом с вами.

В режиме редактирования моба доступны следующие команды:
* alignment <число> — характер
* create <vnum> — синоним edit mobile create <vnum>
* desc дескр. — переводит в состояние редакторирования строки (см. string)
* level <число> — левел
* long <строка> — длинное описание (look в комнате)
* name <строка> — имя (на что отзывается, kill <строка>)
* shop hours <число> <число> — время открытия/закрытия
* shop profit <число> <число> — процент с продажи/покупки
* shop type <число> <строка> — что покупаем (<число> от 0 до 4, <строка> - item type)
* shop delete <число> — удалить что было объявлено в type. (<число> от 0 до 4)
* short <строка> — короткое описание. (<строка> ушел на север.)
* show / '' (пустая строка) — показать текущую инф-ю о мобе.
* spec <строка> — установить спец. процедуру. (см. olchelp)
* sex <пол> — пол (см. olchelp)
* act <флаг> — экт (см. olchelp)
* affect <флаг> — аффекты (см. olchelp)
* detection <флаг> — детекты (см. olchelp)
* armor [ac-pierce [ac-bash [ac-slash [ac-exotic]]]] — защита
* form <флаг> — форма (см. olchelp)
* part <флаг> — части тела (см. olchelp)
* imm <флаг> — иммунности (см. olchelp)
* res <флаг> — стойкости (см. olchelp)
* vuln <флаг> — уязвимости (см. olchelp)
* material <строка> — материал (см. olchelp)
* off <флаг> — атакующие способности (см. olchelp)
* size <число> — размер (см. olchelp)
* hitdice <число>d<число>+<число> — hp
* manadice <число>d<число>+<число> — mana
* damdice <число>d<число>+<число> — damage
* race <строка> — раса (см. olchelp)
* position start <позиция> — стартовая позиция (см. olchelp)
* position default <позиция> — нормальная позиция (см. olchelp)
* wealth <число> — богатство
* group <число> — группа
* practicer <group> — может практиковать группу (см. olchelp)
* hitroll <число> — хитролл
* damtype <damtype> — тип атаки (см. olchelp)
* list — список комнат где ресетится.

## oedit — редактор объектов
Команда для редактирования объектов.
* edit object <vnum> — редактировать объект с номером <vnum>. Переводит редактор в состояние редактирования объекта.
* edit object create <vnum> — создать нового моба с номером <vnum>. Переводит редактор в состояние редактирования объекта.
* edit object show <vnum> — показать информацию об объекте с номером <vnum>.
* edit object load <vnum> — загрузить объект с номером <vnum>. Объект попадет к вам в руки, а если у него нет WEAR_TAKE, то в комнату.

В режиме редактирования объекта доступны следующие команды:
* show или '' (пустая строка) — показать информацию о редактируемом объекте.
* create <vnum> — синоним edit object create <vnum>
* addaffect <apply> <число> [<affwhere> <флаги>] — добавить аффект. см. olchelp
* delaffect <число> — удалить аффект с индексом <число>
* ed ... — см. extradescr
* cost <число> — цена
* long <строка> — длинное описание (look в комнате)
* name <строка> — на что откликается (take <строка)
* short <строка> — короткое описание (Ты вворужешься <строка>)
* v0, v1, v2, v3, v4 — значения, зависящие от типа объекта (*)
* weight <число> — вес
* extra <флаг> — экстра-флаги объекта (см. olchelp)
* wear <wear-loc> — куда одевается (см. olchelp)
* type <тип> — тип объекта
* material <строка> — из чего сделано
* level <число> — уровень
* limit <число> — лимит (-1 не лимитный шмот)
* condition <число> — состояние. 100 - превосходное. 0 - ужасное
* list — в каких комнатах ресетится
(*): Для каждого типа объекта. Для того, чтоб узнать какие из v[0-4] что означают, можно установить необходимый тип и сделать show.

## resets: выставление ресетов в комнате
Команда редактирования списка ресетов комнаты. Порядок ресетов имеет значение: если один объект должен попасть в другой, или в инвентарь к мобу, то моб, или контейнер должен быть описан первым. Порядок определяется индексом <index>.
* resets <index> obj <vnum> <wear-loc> — добавить объект <vnum> в список ресетов инвентаря последнего моба. <wear-loc> определяе куда должен быть одет объект.
* resets <index> obj <vnum> inside <vnum2> [limit] [count] — добавить объект <vnum> в список ресетов внутри объекта <vnum2>. максимальное число объектов в мире - [limit]. в контейнере - [count].
* resets <index> obj <vnum> room — добавить объект <vnum> в список ресетов комнаты (на полу).
* resets <index> mob <vnum> [max_world] [max_room] — добавить моб <vnum> в список ресетов комнаты. число мобов в мире - [max_world]. в комнате - [max_room]
* resets <index> delete — удалить ресет с индексом <index>.

## olchelp: справка по таблицам флагов
Посмотреть возможные значения флагов и перечислимых типов (размер, аффект, пол, спелы, и т.п.)
* olchelp — без параметров показывает список названий таблиц и их описание.
* olchelp <таблица> [...] — показывает возможные значения элементов таблицы. некоторые таблицы предполагают возможность ограничить вывод дополнительными ограничениями (olchelp spells defend).

## ed: устаревший редактор длинных описаний
Команда для редактирования дополнительных описаний комнат/объектов. Доступна исключительно из состояний редактирования комнат и объектов.
* ed add <строка> — добавить описание с ключом <строка>. переводит редактор в состояние редактирования строки (см. string)
* ed edit <строка> — изменить описание с ключом <строка>. переводит редактор в состояние редактирования строки (см. string)
* ed delete <строка> — удаляет описание с ключом <строка>.
* ed format <строка> — форматирует описание с ключом <строка>.
Полная справка: https://github.com/dreamland-mud/dreamland_code/wiki/OLC:-ED-editor-(obsolete)

## security
Устанавливает уровень привелегии чара в <число>. Чару необходимо иметь уровень привелегий больше 100, чтоб иметь право устанавливать привелегии другим.

## olcvnum.
* olcvnum <player> show
* olcvnum <player> set <min_vnum> <max_vnum> <security level>
* olcvnum <player> del <min_vnum> <max_vnum>