<h1>Поваренная книга, или справочник по OLC </h1>
<em>(оригинал этой книги находится в мире в библиотеке на Улице Вязов)</em>
<p />
<p />
Старинная ужасно потрепаная рукопись. По всей видимости, ее не раз варили.
Чернила растеклись, так что явно половина рецептов утрачена.
На обложке красуется надпись большими зелеными буквами O L C.
Чуть ниже кривым почерком 'здесь был Филдс'.
В конце книги есть еще пору свободных страниц. Вероятно для того, чтобы
всегда было можно ее дополнить.
<p />
Если есть свободное время и нечего больше читать, эта книга поможет тебе
узнать кое-что новое о мире кулинарии. Начинай прямо с введения (см. <a href="#XreFintro" class="twikiCurrentTopicLink twikiAnchorLink">intro</a>).
<p />
<div class="twikiToc"> <ul>
<li> <a href="#Введение"> Введение</a> </li>
<li> <a href="#Команда vnumck."> Команда vnumck.</a>
</li> <li> <a href="#Команда trap."> Команда trap.</a>
</li> <li> <a href="#Редактор строк."> Редактор строк.</a>
</li> <li> <a href="#Команда security."> Команда security.</a>
</li> <li> <a href="#Команда olcvnum."> Команда olcvnum.</a>
</li> <li> <a href="#Команда edit room.      (redit)"> Команда edit room.      (redit)</a>
</li> <li> <a href="#Команда resets."> Команда resets.</a>
</li> <li> <a href="#Команда olchelp."> Команда olchelp.</a>
</li> <li> <a href="#Команда edit object."> Команда edit object.            (oedit)</a>
</li> <li> <a href="#Команда edit mobile."> Команда edit mobile.            (medit)</a>
</li> <li> <a href="#Команда ed."> Команда ed.</a>
</li> <li> <a href="#Команды east, north, south, west"> Команды east, north, south, west, up, down.</a>
</li> <li> <a href="#Команда eexit."> Команда eexit.</a>
</li> <li> <a href="#Команда edit."> Команда edit.</a>
</li> <li> <a href="#Команда asave."> Команда asave.</a>
</li> <li> <a href="#Команда edit area.      (aedit)"> Команда edit area.      (aedit)</a>
</li> <li> <a href="#Команда alist."> Команда alist.</a>
</li></ul> 
</li></ul> 
</div>
<p />
<a name="XreFintro"></a> 
<h1><a name="Введение"></a> Введение </h1>
<p />
Плагин, реализующий набор команд, предназначеных для редактирования арий.
<p />
В различных состояниях набор команд отличается.
<p />
Обычно командный интерпретатор находится в начальном состоянии.
В этом состоянии доступны все стандартные команды.
<p />
OLC реализует следующие команды доступные в начальном состоянии: <ul>
<li> edit     (см. <a href="#XreFedit" class="twikiCurrentTopicLink twikiAnchorLink">edit</a>)
</li> <li> resets   (см. <a href="#XreFresets" class="twikiCurrentTopicLink twikiAnchorLink">resets</a>)
</li> <li> alist    (см. <a href="#XreFalist" class="twikiCurrentTopicLink twikiAnchorLink">alist</a>)
</li> <li> security (см. <a href="#XreFsecurity" class="twikiCurrentTopicLink twikiAnchorLink">security</a>)
</li> <li> olcvnum  (см. <a href="#XreFolcvnum" class="twikiCurrentTopicLink twikiAnchorLink">olcvnum</a>)
</li> <li> vnumck   (см. <a href="#XreFvnumck" class="twikiCurrentTopicLink twikiAnchorLink">vnumck</a>)
</li> <li> olchelp  (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> asave    (см. <a href="#XreFasave" class="twikiCurrentTopicLink twikiAnchorLink">asave</a>)
</li></ul> 
<p />
При переходе из одного состояния в другое, предыдущее запоминается.
В каждом из состояний предусмотрена возможность вернуться к предыдущему.
К примеру, не завершив редактирования комнаты, можно начать редактировать
объект. После завершения завершения редактирования объекта, продолжится
редактирование комнаты.
<p />
<p />
<a name="XreFvnumck"></a> 
<h2><a name="Команда vnumck."></a> Команда vnumck. </h2>
<p />
Синтаксис: <ul>
<li> vnumck
</li></ul> 
<p />
Проверить диапазон выделеных/используемых vnumов.
<p />
Формат вывода: <ul>
<li> <code>&lt;min_vnum&gt;-&lt;max_vnum&gt;: gap (&lt;gap_size&gt;)</code> свободный промежуток между эриями
</li> <li> <code>&lt;min_vnum&gt;-&lt;max_vnum&gt;: &lt;area_credits&gt;: used: &lt;used_min&gt;-&lt;used_max&gt;</code> выделеный блок vnumов с <code>&lt;min_vnum&gt;</code> по <code>&lt;max_vnum&gt;</code>, из которых 
</li></ul> 
эрией <code>&lt;area_credits&gt;</code> занято только с <code>&lt;used_min&gt;</code> по <code>&lt;used_max&gt;</code>. 
<p />
OLC устроено так, что нарушить границы выделения не удастся.
Данная команда предназначена для проверки целостности диапазонов в случае,
если эрия добавляется ввиде файла.
<p />
<p />
<a name="XreFtraps"></a> 
<h2><a name="Команда trap."></a> Команда trap. </h2>
<p />
Комада более не поддерживается.
Все ловушки описываются с помощью <a href="about:blank">Fenia</a>.
<p />
<p />
<a name="XreFstring"></a> 
<h2><a name="Редактор строк."></a> Редактор строк. </h2>
<p />
Рекдактор строк похож на юниксовую команду ed. Хелп <a href="ed.html">тут</a>.
<p />
<p />
<a name="XreFsecurity"></a> 
<h2><a name="Команда security."></a> Команда security. </h2>
<p />
Синтаксис: <ul>
<li> security &lt;player&gt; &lt;число&gt;
</li></ul> 
<p />
Устанавливает уровень привелегии чара в <code>&lt;число&gt;</code>.
Чару необходимо иметь уровень привелегий больше 100, чтоб иметь право
устанавливать привелегии другим.
<p />
<p />
<a name="XreFolcvnum"></a> 
<h2><a name="Команда olcvnum."></a> Команда olcvnum. </h2>
<p />
Синтаксис: <ul>
<li> olcvnum &lt;player&gt; show
</li> <li> olcvnum &lt;player&gt; set &lt;min_vnum&gt; &lt;max_vnum&gt; &lt;security level&gt;
</li> <li> olcvnum &lt;player&gt; del &lt;min_vnum&gt; &lt;max_vnum&gt;
</li></ul> 
<p />
Устанавливает диапазоны внумов, которые может редактировать игрок.
Для этих внумов ему будут разрешены medit, oedit, redit, asave area.
Команда доступна чарам с уровнем привилегий больше 100.
NB: поле <code>&lt;security level&gt;</code> обязательно, но пока нигде не используется.
<p />
<a name="XreFroom"></a> 
<h2><a name="Команда edit room.      (redit)"></a> Команда edit room.      (redit) </h2>
<p />
Синтаксис: <ul>
<li> edit room
</li> <li> edit room &lt;vnum&gt;
</li> <li> edit room create &lt;vnum&gt;
</li> <li> edit room reset
</li> <li> edit room show &lt;vnum&gt;
</li></ul> 
<p />
Команда для редактирования комнат.
<p />
<table cellspacing="0" id="table1" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>edit room</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> переводит редактор в состояние редактирования комнаты в которой находится чар. </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>edit room &lt;vnum&gt;</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> переводит редактор в состояние редактирования комнаты с номером <code>&lt;vnum&gt;</code>. </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>edit room create &lt;vnum&gt;</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> создает новую комнату с номером <code>&lt;vnum&gt;</code> и переводит редактор в состояние редактирования новосозданной комнаты. </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>edit room create next</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> создает новую комнату со следующим свободным внумом </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>edit room reset</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> обновляет редактируемую комнату. </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> <code>edit room show &lt;vnum&gt;</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol twikiLast"> показывает информацию о комнате <code>&lt;vnum&gt;</code> </td>
		</tr>
	</tbody></table>
<p />
Следующие команды доступны только из состояния редактирования комнаты:
<table cellspacing="0" id="table2" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> create &lt;vnum&gt;  </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> синоним edit room create <code>&lt;vnum&gt;</code> </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> create next        </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> синоним edit room create next </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> desc               </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> устанавливает описание, переводит редактор в состояние редактирования строки (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>) </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> ed ...             </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> см. <a href="#XreFextradescr" class="twikiCurrentTopicLink twikiAnchorLink">extradescr</a> </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> format             </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> форматировать описание </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> name &lt;строка&gt;</pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> установить имя комнаты </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> show               </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> показать инф-ю о редактируемой комнате </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> '' (пустая строка) </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> синоним show </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> heal &lt;число&gt; </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> скорость восстанавления hp.  100 - нормальная скорость </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> mana &lt;число&gt; </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> скорость восстанавления mana.  100 - нормальная скорость </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> north, south, ...  </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> см. <a href="#XreFexits" class="twikiCurrentTopicLink twikiAnchorLink">exits</a> </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> owner &lt;строка&gt;</pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> установить владельца </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> clan &lt;строка&gt;</pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> установить клан </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> pstore &lt;vnum&gt;</pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> установить vnum хранилища животных (если не следующий) </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> eexit ...          </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> см. <a href="#XreFeexits" class="twikiCurrentTopicLink twikiAnchorLink">eexits</a> </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> trap ...           </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> см. <a href="#XreFtraps" class="twikiCurrentTopicLink twikiAnchorLink">traps</a> </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> mroom add &lt;level&gt; &lt;vnum&gt;</pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> добавить мультирум.                               чары с левелом ниже <code>&lt;level&gt;</code> будут попадать в комнату <code>&lt;vnum&gt;</code> </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> mroom delete &lt;level&gt;</pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> удалить мультирум. </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> mlist &lt;all/имя&gt;     </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> список мобов в этой эрии (all - всех, имя - с заданым именем) </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> olist &lt;all/тип/имя&gt; </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> список объектов в этой эрии (                               <code>all</code> - всех,                               <code>тип</code> - с заданым типом (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>),                               <code>имя</code> - с заданым именем) </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> rlist               </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> список комнат в этой эрии </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> &lt;room_flags&gt;  </pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a> </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> &lt;sector_type&gt; </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a> </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> mreset &lt;vnum&gt; &lt;mr&gt; &lt;mw&gt;</pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> добавить ресет моба в редактируемой комнате.                         <code>&lt;vnum&gt;</code> - номер моба,                          <code>&lt;mr&gt;</code> - максимум мобов в комнате,                         <code>&lt;mw&gt;</code> - максимум мобов в мире. (*) </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> oreset &lt;vnum&gt;  </pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> добавить ресет объекта в редактируемой комнате.                         <code>&lt;vnum&gt;</code> - номер объекта. объект будет ресетиться                         на полу. (*) </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <pre> oreset &lt;vnum&gt; &lt;строка&gt;</pre> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> добавить ресет объекта в редактируемой комнате.                         <code>&lt;vnum&gt;</code> - номер объекта.                         <code>&lt;строка&gt;</code> - имя объекта, в котором должен ресетиться                         добавляемы. (*) </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> <pre> oreset &lt;vnum&gt; &lt;строка&gt; &lt;wear-loc&gt;</pre> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol twikiLast"> добавить ресет объекта в редактируемой                         комнате. объект с номером <code>&lt;vnum&gt;</code> будет надет на                          моба с именем <code>&lt;строка&gt;</code> на wear location                         <code>&lt;wear-loc&gt;</code>. (*) (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>) </td>
		</tr>
	</tbody></table>
<p />
Команды отмеченые как (*) детально не отлаживались т.к. существует аналог
с более широкой функциональностью (см. <a href="#XreFresets" class="twikiCurrentTopicLink twikiAnchorLink">resets</a>).
<p />
<p />
<a name="XreFresets"></a> 
<h2><a name="Команда resets."></a> Команда resets. </h2>
<p />
Синтаксис: <ul>
<li> resets
</li> <li> resets &lt;index&gt; obj &lt;vnum&gt; &lt;wear-loc&gt;
</li> <li> resets &lt;index&gt; obj &lt;vnum&gt; inside &lt;vnum&gt; [limit] [count]
</li> <li> resets &lt;index&gt; obj &lt;vnum&gt; room
</li> <li> resets &lt;index&gt; mob &lt;vnum&gt; [max_world] [max_room]
</li> <li> resets &lt;index&gt; delete
</li></ul> 
<p />
Команда редактирования списка ресетов комнаты.
Порядок ресетов имеет значение: если один объект должен попасть в другой,
или в инвентарь к мобу, то моб, или контейнер должен быть описан первым.
Порядок определяется индексом &lt;index&gt;.
<p /> <ul>
<li> resets &lt;index&gt; obj &lt;vnum&gt; &lt;wear-loc&gt;    добавить объект &lt;vnum&gt; в список ресетов                                        инвентаря последнего моба. &lt;wear-loc&gt;                                        определяе куда должен быть одет объект.
</li> <li> resets &lt;index&gt; obj &lt;vnum&gt; inside &lt;vnum2&gt; [limit] [count]                                        добавить объект &lt;vnum&gt; в список ресетов                                        внутри объекта &lt;vnum2&gt;. максимальное                                        число объектов в мире - [limit].                                        в контейнере - [count].
</li> <li> resets &lt;index&gt; obj &lt;vnum&gt; room          добавить объект &lt;vnum&gt; в список ресетов                                        комнаты (на полу).
</li> <li> resets &lt;index&gt; mob &lt;vnum&gt; [max_world] [max_room]                                        добавить моб &lt;vnum&gt; в список ресетов                                        комнаты. число мобов в мире -                                        [max_world]. в комнате - [max_room]
</li> <li> resets &lt;index&gt; delete                   удалить ресет с индексом &lt;index&gt;.
</li></ul> 
<p />
<p />
<p />
<a name="XreFolchelp"></a> 
<h2><a name="Команда olchelp."></a> Команда olchelp. </h2>
<p />
Синтаксис: <ul>
<li> olchelp
</li> <li> olchelp &lt;таблица&gt; [...]
</li></ul> 
<p />
Команда для посмотреть возможные значения флагов и перечислимых типов
(размер, аффект, пол, спелы, и т.п.)
<p /> <ul>
<li> olchelp                 без параметров показывает список названий таблиц и их                        описание.
</li> <li> olchelp &lt;таблица&gt; [...] показывает возможные значения элементов таблицы.                        некоторые таблицы предполагают возможность ограничить                        вывод дополнительными ограничениями                        (olchelp spells defend).
</li></ul> 
<p />
<p />
<a name="XreFobject"></a> 
<h2><a name="Команда edit object."></a><a name="Команда edit object.            "></a> Команда edit object.            (oedit) </h2>
<p />
Синтаксис: <ul>
<li> edit object &lt;vnum&gt;
</li> <li> edit object create &lt;vnum&gt;
</li> <li> edit object show &lt;vnum&gt;
</li> <li> edit object load &lt;vnum&gt;
</li></ul> 
<p />
Команда для редактирования объектов.
<p /> <ul>
<li> edit object &lt;vnum&gt;         редактировать объект с номером &lt;vnum&gt;.                           Переводит редактор в состояние редактирования                           объекта.
</li> <li> edit object create &lt;vnum&gt;  создать нового моба с номером &lt;vnum&gt;. Переводит                           редактор в состояние редактирования объекта.
</li> <li> edit object show &lt;vnum&gt;    показать информацию об объекте с номером &lt;vnum&gt;.
</li> <li> edit object load &lt;vnum&gt;    загрузить объект с номером &lt;vnum&gt;. Объект попадет                           к вам в руки, а если у него нет WEAR_TAKE, то в комнату.
</li></ul> 
<p />
В режиме редактирования объекта доступны следующие команды: <ul>
<li> show                 показать информацию о редактируемом объекте.
</li> <li> '' (пустая строка)   синоним show
</li> <li> create &lt;vnum&gt;        синоним edit object create &lt;vnum&gt;
</li> <li> addaffect &lt;apply&gt; &lt;число&gt; [&lt;affwhere&gt; &lt;флаги&gt;]                            добавить аффект.                            см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>
</li> <li> delaffect &lt;число&gt;    удалить аффект с индексом &lt;число&gt;
</li> <li> ed ...               см. <a href="#XreFextradescr" class="twikiCurrentTopicLink twikiAnchorLink">extradescr</a>
</li> <li> cost &lt;число&gt;         цена
</li> <li> long &lt;строка&gt;        длинное описание (look  в комнате)
</li> <li> name &lt;строка&gt;        на что откликается (take &lt;строка)
</li> <li> short &lt;строка&gt;       короткое описание (Ты вворужешься &lt;строка&gt;)
</li> <li> v0, v1, v2, v3, v4   значениея, зависящие от типа объекта (*)
</li> <li> weight &lt;число&gt;       вес
</li> <li> extra &lt;extra&gt;        экстра-флаги объекта (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> wear &lt;wear-loc&gt;      куда одевается (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> type &lt;type&gt;          тип объекта
</li> <li> material &lt;строка&gt;    из чего сделано
</li> <li> level &lt;число&gt;        уровень
</li> <li> limit &lt;число&gt;        лимит (-1 не лимитный шмот)
</li> <li> condition &lt;число&gt;    состояние. 100 - превосходное. 0 - ужасное
</li> <li> list                 в каких комнатах ресетится
</li></ul> 
<p />
(*):
Для каждого типа объекта. Для того, чтоб узнать какие из v[0-4] что означают,
можно установить необходимый тип и сделать show.
<p />
<p />
<a name="XreFmobile"></a> 
<h2><a name="Команда edit mobile."></a><a name="Команда edit mobile.            "></a> Команда edit mobile.            (medit) </h2>
<p />
Синтаксис: <ul>
<li> edit mobile &lt;vnum&gt;
</li> <li> edit mobile create &lt;vnum&gt;
</li> <li> edit mobile show &lt;vnum&gt;
</li> <li> edit mobile load &lt;vnum&gt;
</li></ul> 
<p />
Команда для редактирования мобов.
<p /> <ul>
<li> edit mobile &lt;vnum&gt;          редактировать моба с номером &lt;vnum&gt;. Переводит                            редактор в состояние редактирования мобов.
</li> <li> edit mobile create &lt;vnum&gt;   создать нового моба с номером &lt;vnum&gt;. Переводит                            редактор в состояние редактирования мобов.
</li> <li> edit mobile show &lt;vnum&gt;     показать информацию о мобе с номером &lt;vnum&gt;.
</li> <li> edit mobile load &lt;vnum&gt;     загрузить монстра с номером &lt;vnum&gt;. Монстр                            появится рядом с вами.
</li></ul> 
<p />
В режиме редактирования моба доступны следующие команды: <ul>
<li> alignment &lt;число&gt;                 характер
</li> <li> create &lt;vnum&gt;                     синоним edit mobile create &lt;vnum&gt;
</li> <li> desc                              дескр. переводит в состояние                                         редакторирования строки                                         (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>)
</li> <li> level &lt;число&gt;                     левел
</li> <li> long &lt;строка&gt;                     длинное описание (look  в комнате)
</li> <li> name &lt;строка&gt;                     имя (на что отзывается,                                         kill &lt;строка&gt;)
</li> <li> shop hours &lt;число&gt; &lt;число&gt;        время открытия/закрытия
</li> <li> shop profit &lt;число&gt; &lt;число&gt;       процент с продажи/покупки
</li> <li> shop type &lt;число&gt; &lt;строка&gt;        что покупаем (&lt;число&gt; от 0 до 4,                                         &lt;строка&gt; - item type)
</li> <li> shop delete &lt;число&gt;               удалить что было объявлено в type.                                         (&lt;число&gt; от 0 до 4)
</li> <li> short &lt;строка&gt;                    короткое описание.                                         (&lt;строка&gt; ушел на север.)
</li> <li> show                              показать текущую инф-ю о мобе.
</li> <li> '' (пустая строка)                синоним show
</li> <li> spec &lt;строка&gt;                     установить спец. процедуру.                                         (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> sex &lt;sex&gt;                         пол (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> act &lt;act&gt;                         экт (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> affect &lt;affect&gt;                   аффекты (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> detection &lt;detection&gt;             детекты (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> armor [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]   защита
</li> <li> form &lt;form&gt;                       форма (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> part &lt;part&gt;                       части тела (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> imm &lt;immune&gt;                      иммунности (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> res &lt;resist&gt;                      стойкости (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> vuln &lt;vuln&gt;                       уязвимости (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> material &lt;строка&gt;                 материал (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> off &lt;offensive&gt;                   атакующие способности                                         (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> size &lt;size&gt;                       размер (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> hitdice &lt;число&gt;d&lt;число&gt;+&lt;число&gt;   hp
</li> <li> manadice &lt;число&gt;d&lt;число&gt;+&lt;число&gt;  mana
</li> <li> damdice &lt;число&gt;d&lt;число&gt;+&lt;число&gt;   damage
</li> <li> race &lt;race&gt;                       расса (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> position start &lt;position&gt;         стартовая позиция                                         (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> position default &lt;position&gt;       нормальная позиция                                         (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> wealth &lt;число&gt;                    богатство
</li> <li> group &lt;число&gt;                     группа
</li> <li> practicer &lt;group&gt;                 может практиковать группу                                         (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> hitroll &lt;число&gt;                   хитролл
</li> <li> damtype &lt;damtype&gt;                 тип атаки (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> list                              список комнат где ресетится.
</li></ul> 
<p />
<p />
<p />
<a name="XreFextradescr"></a> 
<h2><a name="Команда ed."></a> Команда ed. </h2>
<p />
Синтаксис: <ul>
<li> ed add &lt;строка&gt;
</li> <li> ed edit &lt;строка&gt;
</li> <li> ed delete &lt;строка&gt;
</li> <li> ed format &lt;строка&gt;
</li></ul> 
<p />
Команда для редактирования дополнительных описаний комнат/объектов.
Достопна исключительно из состояний редактирования комнат и объектов.
<p /> <ul>
<li> ed add &lt;строка&gt;      добавить описание с ключом &lt;строка&gt;. переводит редактор                     в состояниередактирования строки (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>)
</li> <li> ed edit &lt;строка&gt;     изменить описание с ключом &lt;строка&gt;. переводит редактор                     в состояниередактирования строки (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>)
</li> <li> ed delete &lt;строка&gt;   удаляет описание с ключом &lt;строка&gt;.
</li> <li> ed format &lt;строка&gt;   форматирует описание с ключом &lt;строка&gt;.
</li></ul> 
<p />
<p />
<a name="XreFexits"></a> 
<h2><a name="Команды east, north, south, west"></a> Команды east, north, south, west, up, down. </h2>
<p />
dir ::= &lt;east|north|south|west|up|down&gt;
<p />
Синтаксис: <ul>
<li> &lt;dir&gt;
</li> <li> &lt;dir&gt; &lt;exit_flags&gt;
</li> <li> &lt;dir&gt; ?
</li> <li> &lt;dir&gt; delete
</li> <li> &lt;dir&gt; link &lt;vnum&gt;
</li> <li> &lt;dir&gt; dig &lt;vnum&gt;
</li> <li> &lt;dir&gt; dig next 
</li> <li> &lt;dir&gt; room &lt;vnum&gt;
</li> <li> &lt;dir&gt; key &lt;vnum&gt;
</li> <li> &lt;dir&gt; name &lt;строка&gt;
</li> <li> &lt;dir&gt; descr
</li></ul> 
<p />
Команды предназначеные для редактирования стандартных выходов из комнаты.
Доступны только из состояния редактирования комнаты. Исключение составляет
первый вариант команды, который дублируется в стандартном наборе команд.
<p /> <ul>
<li> &lt;dir&gt;                 переместить чара в направлении &lt;dir&gt;. Эта команда не                      изменяет состояние редактора. Вместо этого редактирование                      переходит к комнате в которую попал чар.
</li> <li> &lt;dir&gt; &lt;exit_flags&gt;    изменить флаги выхода (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> &lt;dir&gt; ?               синоним olchelp exit
</li> <li> &lt;dir&gt; delete          удалить двусторонний проход. если проход односторонний,                      или выход с другой стороны ведет в другую комнату,                      удаляется только односотронний проход.
</li> <li> &lt;dir&gt; link &lt;vnum&gt;     создать двусторонний проход с существующей комнатой                      &lt;vnum&gt;.
</li> <li> &lt;dir&gt; dig &lt;vnum&gt;      создать двусторонний проход в новую комнату. созданая                      комната будет иметь номер &lt;vnum&gt;.
</li> <li> &lt;dir&gt; dig next        создать двусторонний проход в новую комнату. созданая                      комната будет иметь следующий свободный номер.
</li> <li> &lt;dir&gt; room &lt;vnum&gt;     создать односторонний проход в существующую комнату                      &lt;vnum&gt;.
</li> <li> &lt;dir&gt; key &lt;vnum&gt;      установить ключ (для двери)
</li> <li> &lt;dir&gt; name &lt;строка&gt;   установить имя выхода (для open door, lock door)
</li> <li> &lt;dir&gt; descr           установить описание выхода (для look  north). переводит                      редактор в состояние редактирования строки.                      (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>)
</li></ul> 
<p />
<p />
<a name="XreFeexits"></a> 
<h2><a name="Команда eexit."></a> Команда eexit. </h2>
<p />
Синтаксис: <ul>
<li> eexit &lt;keyword&gt; delete
</li> <li> eexit &lt;keyword&gt; add
</li> <li> eexit &lt;keyword&gt;
</li></ul> 
<p />
Команда для редактирования экстравыходов. Доступна исключительно из состояния
редактирования комнаты.
<p /> <ul>
<li> eexit &lt;keyword&gt; delete   удаляет экстравыход &lt;keyword&gt;.
</li> <li> eexit &lt;keyword&gt; add      добавляет экстравыход &lt;keyword&gt;. Эта команда переводит                         редактор в состояние редактирования экстравыхода.
</li> <li> eexit &lt;keyword&gt;          редактирует экстравыход &lt;keyword&gt;. Эта команда                         переводит редактор в состояние редактирования                         экстравыхода.
</li></ul> 
<p />
Если название экстравыхода (&lt;keyword&gt;) должно состоять из нескольких слов,
его необходимо брать в кавычки.
<p />
В состоянии редактирования экстравыходов доступны следующие команды: <ul>
<li> show                  показать текущую инф-ю о редактируемом                            экстравыходе.
</li> <li> '' (пустая строка)    синоним show
</li> <li> desc                  редактировать описание. (для look  keyword).                            переводит редактор в состояние редактирования                            строки. (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>)
</li> <li> rdesc                 редактировать описание комнаты. (добавляется                            к описанию комнаты при look, если чар может видеть                            этот экстравыход). переводит редактор в состояние                            редактирования строки. (см. <a href="#XreFstring" class="twikiCurrentTopicLink twikiAnchorLink">string</a>)
</li> <li> name &lt;строка&gt;         изменить ключевое слово.
</li> <li> key &lt;vnum&gt;            установить ключ.
</li> <li> target &lt;vnum&gt;         установить комнату назначения.
</li> <li> from &lt;число1&gt; &lt;число2&gt; &lt;строка&gt; сообщение тем, кто остался в комнате, из                            которой ушел чар. (*)
</li> <li> to &lt;число1&gt; &lt;число2&gt; &lt;строка&gt; сообщение тем, кто находился в комнате, в                            которую пришел чар. (*)
</li> <li> &lt;eexit_flags&gt;         см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>
</li> <li> &lt;size&gt;                установить максимальный размер чара, способного                            пройти в этот экстравыход. (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li></ul> 
<p />
(*):
Сообщение для from формируется таким образом:
<pre>
    &lt;ru&gt; &lt;rt&gt; &lt;short&gt;.
</pre>
<p />
Сообщение для to формируется таким образом:
<pre>
    &lt;rp&gt; &lt;rt&gt; &lt;short&gt;.
</pre>
<p />
здесь: <ul>
<li> &lt;ru&gt; задается &lt;числом1&gt; из команды from,
</li> <li> &lt;rp&gt; - &lt;числом1&gt; из команды to,
</li> <li> &lt;rt&gt; - &lt;числом2&gt; одинакого для from и to,
</li> <li> &lt;short&gt; - &lt;строка&gt; одинакого для from и to.
</li></ul> 
<p />
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
<p />
<p />
Бред какой-то. Это вместо того, чтоб хранить просто строку act_p. \=
Может рыжая переделает када-то.
<p />
<p />
<a name="XreFedit"></a> 
<h2><a name="Команда edit."></a> Команда edit. </h2>
<p />
Синтаксис: <ul>
<li> edit area ...           (aedit ...)
</li> <li> edit room ...           (redit ...)
</li> <li> edit mobile ...         (medit ...)
</li> <li> edit object ...         (oedit ...)
</li></ul> 
<p />
Предназначена для смены состояния редактора.
(переход к редактированию чего либо).
<p />
Для краткости, команда edit имеет ряд синонимов (aedit, redit, medit, oedit).
<p />
В каждом из состояний будут доступны следующие команды: <ul>
<li> version   показать версию OLC
</li> <li> commands  показать список команд доступных в этом состоянии                (кроме обычных команд)
</li> <li> ?         синоним команды olchelp (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li> <li> done      вернуться в предыдущее состояние.
</li></ul> 
<p />
Для подробностей см. <a href="#XreFarea" class="twikiCurrentTopicLink twikiAnchorLink">area</a>, <a href="#XreFroom" class="twikiCurrentTopicLink twikiAnchorLink">room</a>, <a href="#XreFmobile" class="twikiCurrentTopicLink twikiAnchorLink">mobile</a>, <a href="#XreFobject" class="twikiCurrentTopicLink twikiAnchorLink">object</a>.
<p />
<p />
<a name="XreFasave"></a> 
<h2><a name="Команда asave."></a> Команда asave. </h2>
<p />
Синтаксис: <ul>
<li> asave &lt;vnum&gt;
</li> <li> asave list
</li> <li> asave area
</li> <li> asave changed
</li> <li> asave world
</li></ul> 
<p />
Команда для сохранения эрий на диск.
<p /> <ul>
<li> asave &lt;vnum&gt;    сохранить одну эрию с номером &lt;vnum&gt;. номер можно узнать                из alist
</li> <li> asave list      сохранить только список эрий
</li> <li> asave area      сохранить эрию в которой находится чар
</li> <li> asave changed   сохранить измененные эрии
</li> <li> asave world     сохранить все эрии
</li></ul> 
<p />
<p />
<p />
<a name="XreFarea"></a> 
<h2><a name="Команда edit area.      (aedit)"></a> Команда edit area.      (aedit) </h2>
<p />
Синтаксис: <ul>
<li> edit area
</li> <li> edit area &lt;номер&gt;
</li> <li> edit area create
</li></ul> 
<p /> <ul>
<li> edit area           без аргументов переводит редактор в состояние                    редактирования эрии в которой находится чар.
</li> <li> edit area &lt;номер&gt;   переводит редактор в состояние редактирования эрии                    с номером &lt;номер&gt;. Номер существующий эрии можно узнать                    с помощью команд alist. (см. <a href="#XreFalist" class="twikiCurrentTopicLink twikiAnchorLink">alist</a>)
</li> <li> edit area create    создает новую эрию со следующим свободным номером и                    переводит редактор в состояние редактирования                    новосозданной эрии.
</li></ul> 
<p />
Следующие команды доступны только из состояния редактирования эрии: <ul>
<li> age &lt;число&gt;               установить возраст эрии.
</li> <li> builder &lt;чар1 чар2 ...&gt;   установить список редакторов. (*)
</li> <li> create                    синоним edit area create.
</li> <li> filename &lt;строка&gt;         установить имя файла эрии.
</li> <li> name &lt;строка&gt;             установить имя эрии.
</li> <li> reset                     обновить эрию (repop).
</li> <li> security &lt;число&gt;          установить необходимый уровень приврлегий. (*)
</li> <li> show                      показать текущую ин-ю о редактируемой эрии.
</li> <li> '' (пустая строка)        синоним show.
</li> <li> vnum &lt;число&gt; &lt;число&gt;      установить диапазон vnum'ов.
</li> <li> lvnum &lt;число&gt;             установить нижнюю границу диапазона vnum'ов.
</li> <li> uvnum &lt;число&gt;             установить верхнюю границу диапазона vnum'ов.
</li> <li> levels &lt;число&gt; &lt;число&gt;    установить рекомендуемый диапазон левелов.
</li> <li> credits &lt;строка&gt;          установить строку описания.
</li> <li> resetmsg &lt;строка&gt;         установить reset message.
</li> <li> flag &lt;area_flags&gt;         установить флаги (см. <a href="#XreFolchelp" class="twikiCurrentTopicLink twikiAnchorLink">olchelp</a>)
</li></ul> 
<p />
(*): не поддерживается из-за совместимости со старым форматом эрий.
<p />
<a name="XreFalist"></a> 
<h2><a name="Команда alist."></a> Команда alist. </h2>
<p />
Синтаксис: <ul>
<li> alist
</li></ul> 
<p />
Команда для посмотреть номера эрий. (см. <a href="#XreFarea" class="twikiCurrentTopicLink twikiAnchorLink">area</a>)
<p />