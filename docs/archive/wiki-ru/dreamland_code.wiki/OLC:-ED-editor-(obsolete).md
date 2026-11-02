<h1>Ed, man! man ed! </h1>
<p>
Это новый(*) текстовый редактор, который используется теперь вместо устаревшего
OLCшного. Синтаксис позаимствован из стандартного юниксового строчного редактора
<code>ed</code>, потому многим, привыкшим к командной строке <code>vim</code>, будет удобно.
</p>

> **Примечание** 'новым' этот редактор был на момент написания руководства в 2004м году. Сейчас гораздо удобнее использовать команду `вебредактор` в веб-клиенте, подробности [здесь](https://github.com/dreamland-mud/dreamland_code/wiki/%D0%A0%D0%B5%D0%B4%D0%B0%D0%BA%D1%82%D0%B8%D1%80%D0%BE%D0%B2%D0%B0%D0%BD%D0%B8%D0%B5-%D0%BE%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%B8%D0%B9-%D0%BA%D0%BE%D0%BC%D0%BD%D0%B0%D1%82-%D0%B8-%D0%BF%D1%80%D0%BE%D1%82%D0%BE%D1%82%D0%B8%D0%BF%D0%BE%D0%B2).

<hr/>
<div class="twikiToc"> <ul>
<li> <a href="#Коментарии к этой доке"> Коментарии к этой доке</a> <ul>
<li> <a href="#Специальные обозначания"> Специальные обозначания</a> <ul>
<li> <a href="#=рег.выр="> рег.выр</a>
</li> <li> <a href="#=метка="> метка</a>
</li> <li> <a href="#=рег="> рег</a>
</li> <li> <a href="#=число="> число</a>
</li></ul> 
</li></ul> 
</li> <li> <a href="#Строковый редактор"> Строковый редактор</a> <ul>
<li> <a href="#Режим ввода команд"> Режим ввода команд</a> <ul>
<li> <a href="#=адрес="> адрес</a>
</li> <li> <a href="#=имя_команды="> имя_команды    </a> <ul>
<li> <a href="#=(+1)=          &quot;безымянная&quot; ком">   (+1)          "безымянная" команда . </a>
</li> <li> <a href="#=(.,.)p=        напечатать блок.">   (.,.)p        напечатать блок. </a>
</li> <li> <a href="#=(.,.)n=        листинг.">   (.,.)n        листинг.</a>
</li> <li> <a href="#=(.,.)d[рег]=   удалить блок опр">   (.,.)d[рег]   удалить блок определяемый диапазоном адресов. </a>
</li> <li> <a href="#=(.)a=          добавить текст,">   (.)a          добавить текст, после указанного адреса. </a>
</li> <li> <a href="#=(.)i=          добавить текст,">   (.)i          добавить текст, до указанного адреса.</a>
</li> <li> <a href="#=(.,.)c[рег]=   объединяет в себ">   (.,.)c[рег]   объединяет в себе две команды "d" и "i". </a>
</li> <li> <a href="#=(.)mметка=   пометить позицию.">   (.)m&lt;метка&gt;   пометить позицию.</a>
</li> <li> <a href="#=(.)kметка=   синоним команды &quot;m">   (.)k&lt;метка&gt;   синоним команды "m".</a>
</li> <li> <a href="#=(.,+1)j=       Объединить строк">   (.,+1)j       Объединить строки блока. </a>
</li> <li> <a href="#=(.,.)!стр=   Выполнить &quot;внешнюю">   (.,.)!&lt;стр&gt;   Выполнить "внешнюю" команду стр.</a> <ul>
<li> <a href="#=justify= Форматировать по ширин"> justify Форматировать по ширине</a>
</li> <li> <a href="#=show=  показать блок в цвете."> show  показать блок в цвете.</a>
</li></ul> 
</li> <li> <a href="#=(.,.)Y[рег]=   Копировать (yank">   (.,.)Y[рег]   Копировать (yank).</a>
</li> <li> <a href="#=(.)P[рег]=     Вставить содержи">   (.)P[рег]     Вставить содержимое регистра рег после указанного адреса.</a>
</li> <li> <a href="#=(.,.)s[парам]= Найти и заменить">   (.,.)s[парам] Найти и заменить подстроку в указанном диапазоном адресов блоке. </a>
</li> <li> <a href="#=u=             Откат.">   u             Откат.</a>
</li> <li> <a href="#=q=             Завершить редакт">   q             Завершить редактирование.</a>
</li></ul> 
</li></ul> 
</li> <li> <a href="#Режим построчного ввода"> Режим построчного ввода</a>
</li></ul> 
</li></ul> 
</div>
<p />
<h2><a name="Коментарии к этой доке"></a> Коментарии к этой доке </h2>
<p />
Выражения в различных скобках означают когда их писать обязательно, а
когда нет, и что будет, если их не написать:
<table cellspacing="0" id="table1" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>&lt;bubu&gt;</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> в этом месте обязательно должен присутствовать <code>bubu</code>.                            если ничего не написать на этом месте, то будет ошибка. </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>[bubu]</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> <code>bubu</code> можно не писать, что тогда будет - читать в                                 коментариях к этой конструкции. </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> <code>(bubu)</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol twikiLast"> если не написать ничего, то работать будет так, будто                                    написано <code>bubu</code>. </td>
		</tr>
	</tbody></table>
<p />
Например: запись <code>(.,.)p</code> означает, что команда <code>p</code> это то же самое что и <code>.,.p</code>
<p />
<h3><a name="Специальные обозначания"></a> Специальные обозначания </h3>
Следующие обозначения используются при описании синтаксиса команд
<p />
<h4><a name="=рег.выр="></a> <code>рег.выр</code> </h4>
Регулярное выражение (regular expression). Используется для поиска по шаблону. 
Как правильно шаблоны писать в этой доке не описывается.
<p />
<h4><a name="=метка="></a> <code>метка</code> </h4>
Обозначается одним символом. Метка. Используется для запоминания адресов строк.
<p />
<h4><a name="=рег="></a> <code>рег</code> </h4>
Регистр обозначается одим символом. Содержит блок строк.
Используется для копирования блоков.
<p />
<h4><a name="=число="></a> <code>число</code> </h4>
Последовательность цифр от 0 до 9. 
Представляет собой десятичное число в традиционной записи 
<p />
<p />
<h2><a name="Строковый редактор"></a> Строковый редактор </h2>
<p />
Редактор может быть в двух состояниях (режимах).
В первом вводятся команды, во втором построчно вводится текст. 
Состояние редактора отображается в промпте.
При старте редактор находится в режиме ввода команд. 
<p />
<p />
<h3><a name="Режим ввода команд"></a> Режим ввода команд </h3>
В командном режиме промпт состоит из символа <code>:</code> (двоеточие).
Команды редактора имеют следующий общий синтаксис:
<p />
   <code>[адрес[,адрес]]имя_команды[параметры]</code>
<p />
<h4><a name="=адрес="></a> <code>адрес</code> </h4>
адрес можно считать номером строки в редактируемом тексте.
         Указать адрес можно указать одним из следующих способом:
<table cellspacing="0" id="table2" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>&lt;число&gt;</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> порядковый номер строки в редактируемом тексте, начиная с 1. </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>.</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> текущая строка. </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>+&lt;число&gt;</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> порядковый номер строки, начиная с текущей.                                                    (<code>+1</code> - следующая строка, <code>+2</code> - после-следующая, и т.д.) </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>-&lt;число&gt;</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> порядковый номер строки в обратную сторону, начиная с текущей.                       (<code>-1</code> - предыдущая строка, <code>-2</code> - пред-предыдущая, и т.д.) </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>$</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> последняя строка </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>'&lt;метка&gt;</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol"> строка помеченная символом <code>метка</code> </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> <code>/&lt;рег.выр&gt;/</code> </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> следующая строка, найденная регулярным выражениием <code>рег.выр</code>.                          Поиск начинается со следующей за текущей строки. </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> <code>?&lt;рег.выр&gt;?</code> </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol twikiLast"> предыдущая строка, найденная регулярным выражением <code>рег.выр</code>.                          Поиск начинается с предыдущей строки, относительно текущей. </td>
		</tr>
	</tbody></table>
<p />
   В случае, если <code>&lt;рег.выр&gt;</code> пустое, оно считается равным последнему 
   используемому. Если адрес указываемый с помощью символов <code>/</code>,  или <code>?</code> 
   единственный, а используемая команда безымянна (см ниже список комад),
   второй символ <code>/</code>, или <code>?</code> указывать не обязательно. То есть, строка
   содержащая только символ <code>/</code> - правильная команда редактора и означает
   <em>найти, показать и сделать текущей следующую строку удовлетворяющую последнему указанному регулярному выражению</em>.
<p />
   Кроме этого, над адресами действуют арифметические операции <code>+</code> <em>(плюс)</em> и
   <code>-</code> <em>(минус)</em>. Это значит, что если <code>адрес</code> это выражение определенное 
   любым из указанных выше правил, то выражение <code>&lt;адрес&gt;+&lt;число&gt;</code> и 
   <code>&lt;адрес&gt;-&lt;число&gt;</code> - тоже правильный адрес строки. То есть, выражение
   <code>$-1</code> означает предпоследнюю строку в редактируемом тексте, а
   <code>/test/+1</code> - следующую за содержащей слово <em>test</em> строку.
<p />
   Многие команды могут оперировать с блоком строк. 
   В этом случае такой команде можно передавать два адреса: 
   <strong>первый</strong> - адрес первой строки блока и <strong>второй</strong> - адрес последней строки блока.
<p />
   Для краткости пару адресов <code>1,$</code> <em>(весь текст с первой строки по последнюю)</em>
   можно обозначить символом <code>%</code> <em>(процент)</em>.
<p />
<h4><a name="=имя_команды="></a> <code>имя_команды</code> </h4>
Команда - один символ.
   Вот список всех команд и объяснение значений их параметров. В скобках
   перед именем команды указано значение по умолчанию для адресов, 
   передаваемых команде. Плюсы и двоеточия в примерах обозначают текущий 
   промпт. Писать их не надо.  Строки не начинающиеся с символов промпта 
   отражают вывод редактора. Их тоже не надо писать.
<p />
<h5><a name="=(+1)=          &quot;безымянная&quot; ком"></a> <code>(+1)</code>          "безымянная" команда . </h5>
принимает единственный адрес, печатает строку по этому адресу и делает ее текущей.  
Так, к примеру, команда <strong>/test</strong> означает
<em>найти, показать и сделать текущей строку, содержащую слово test</em>.
Как указано в скобках, если этой команде не передать
ядрес явно, используется адрес следующей за текущей
строкой. То есть, если просто давить <code>enter</code>, можно 
построчно листать содержимое. 
<p />
<h5><a name="=(.,.)p=        напечатать блок."></a> <code>(.,.)p</code>        напечатать блок. </h5>
Принимает в параметры два адреса - начало и конец блока, который необходимо напечатать. 
Последняя напечатанная строка становится текущей. 
Если адреса не указывать, команда <strong>p</strong> напечатает текущую строку, что и 
соответствует блоку, определяемуму парой адресов <code>.,.</code>. 
<p />
<h5><a name="=(.,.)n=        листинг."></a> <code>(.,.)n</code>        листинг. </h5>
аналог комнды <code>p</code>, с тем исключением, что команда <code>n</code> 
предваряет каждую печатаемую строку ее номером, относительно
начала редактируемого текста.
<p />
<h5><a name="=(.,.)d[рег]=   удалить блок опр"></a> <code>(.,.)d[рег]</code>   удалить блок определяемый диапазоном адресов. </h5>
Содержимое
удаленного блока сохраняется в регистре <code>рег</code>. Если <code>рег</code>
не указан, используется регистр поумолчанию. Строка 
следующая за последней удаленной становится текущей.
Аналогично <code>p</code>, если не указывать адресов, удаляет текущую
строку.
<p />
		 Пример:
<blockquote><pre>
: 1d        -- удалить первую строку.
</pre></blockquote>
<p />
		 Еще пример см. ниже.
<p />
<h5><a name="=(.)a=          добавить текст,"></a><a name="=(.)a=          добавить текст, "></a> <code>(.)a</code>          добавить текст, после указанного адреса. </h5>
Эта команда переводит редактор в режим построчного ввода (см. начало этого хелпа). 
Чтоб завершить ввод добавляемого текста, введите строку состоящую из одной точки. 
Последняя добавленная строка становится текущей.
<p />
Пример:
Пускай текст содержит:
<blockquote><pre>
1
2
3
</pre></blockquote>
<p />
Выполним:
<blockquote><pre>
: $a                - добавить текст после последней строки.
+ bebe              - добавляем строку bebe
+ mumu              - добавляем строку mumu
+ .                 - конец ввода
</pre></blockquote>
<p />
Тепер текст выглядит так:
<blockquote><pre>
1
2
3
bebe
mumu
</pre></blockquote>
<p />
<h5><a name="=(.)i=          добавить текст,"></a><a name="=(.)i=          добавить текст, "></a> <code>(.)i</code>          добавить текст, до указанного адреса. </h5>
Аналог команды <code>a</code> с тем отличием, что введенный текст вставляется не после, а перед указанным адресом.
<p />
Пример:
Пускай текст содержит:
<blockquote><pre>
1
2
3
</pre></blockquote>
<p />
Выполним:
<blockquote><pre>
: $i                - вставить текст перед последней строкой.
+ bebe              - добавляем строку bebe
+ mumu              - добавляем строку mumu
+ .                 - конец ввода
</pre></blockquote>
<p />
Тепер текст выглядит так:
<blockquote><pre>
1
2
bebe
mumu
3
</pre></blockquote>
<p />
<h5><a name="=(.,.)c[рег]=   объединяет в себ"></a> <code>(.,.)c[рег]</code>   объединяет в себе две команды "d" и "i". </h5>
Команде <code>d</code> передаются те же адреса, что и <code>c</code>, а <code>i</code> - следующая строка за последней удаленной. 
Удаленный текст запоминается в регистре <code>рег</code>, как и в команде <code>d</code>. 
Таким образом, команда <code>c</code> заменяет переданный ей блок на введенный пользователем
в режиме построчного ввода. Последняя добавленная строка становится текущей.
<p />
Пример:
<blockquote><pre>
: %c             - заменить весь редактируемый текст на ...
+ meme           - строку meme ...
+ .              - и все.
</pre></blockquote>
<p />
<h5><a name="=(.)mметка=   пометить позицию."></a> <code>(.)m&lt;метка&gt;</code>   пометить позицию. </h5>
Присваивает метке <code>символ</code> указанный команде адрес. Адрес текущей строки не модифицируется.
<p />
Пример: Пускай текст содержит:
<blockquote><pre>
0
&gt;&gt;bu&lt;&lt;
8
7
&lt;&lt;bu&gt;&gt;
5
</pre></blockquote>
<p />
Выполним:
<blockquote><pre>
: 1                - показать и перейти к первой строке 0
: /bu              - поиском вперед найти, показать и сделать текущей строку содержащую bu &gt;&gt;bu&lt;&lt;
: mq               - присвоить текущей строке метку q
: /                - продолжить поиск bu со следующей строки &lt;&lt;bu&gt;&gt;
: 'q,.d            - удалить строки начиная с &gt;&gt;bu&lt;&lt; и заканчивая &lt;&lt;bu&gt;&gt;
</pre></blockquote>
<p />
<h5><a name="=(.)kметка=   синоним команды &quot;m"></a> <code>(.)k&lt;метка&gt;</code>   синоним команды "m". </h5>
<p />
<p />
<p />
<h5><a name="=(.,+1)j=       Объединить строк"></a> <code>(.,+1)j</code>       Объединить строки блока. </h5>
Заменяет в блоке переводы строки на пробелы. Склеенная строка становится текущей.
<p />
Пример: Пускай редактируемый текст содржит:
<blockquote><pre>
9
8
7
6
5
4
</pre></blockquote>
<p />
Выполним:
<blockquote><pre>
: /8/,/6/j     - склеить строки начиная со втрой и кончая четвертой
</pre></blockquote>
<p />
Текст после выполнения команды:
<blockquote><pre>
9
8 7 6
5
4
</pre></blockquote>
<p />
<p />
<h5><a name="=(.,.)!стр=   Выполнить &quot;внешнюю"></a> <code>(.,.)!&lt;стр&gt;</code>   Выполнить "внешнюю" команду <code>стр</code>. </h5>
Внешней команде передется в парамаетры
содержимое блока определяемого диапазоном адресов. 
Результат выполнения вставляется на место
указанного блока.
<p />
Для фени <em>(редактор, вызываемый методом персонажа edit( ))</em>
<code>стр</code> интерпретируется как выражение языка фени. 
На момент вычисления выражения контекст содержит переменные
<code>ch</code> (персонаж, выполняющий команду редактора) и <code>text</code> 
(строка, содержащая исходный вариант блока). Результат
вычисления выражения используется как результат выполнения
внешней команды.
<p />
Для тексового редактора вызванного из <a href="olc.html">OLC</a> существует только 
две специфические внешние команды. Как видно из примеров, имена этих команд можно сокращать.
Все остальные варианты <code>стр</code> обрабатываются как обычные игровые команды,
блок при этом не модифицируется.
<p />
<h6><a name="=justify= Форматировать по ширин"></a> <code>justify</code> Форматировать по ширине </h6>
Форматирует абзац текста по ширине. 
<p />
Команде justify можно передать два параметра:
<table cellspacing="0" id="table3" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> # </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1"> значение </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol2 twikiLastCol"> по умолчанию </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol"> 1 </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1"> количество пробелов в начале абзаца </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol2 twikiLastCol"> 8 </td>
		</tr>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> 2 </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLast"> ширина абзаца в символах </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol2 twikiLastCol twikiLast"> 70 </td>
		</tr>
	</tbody></table>
<p />
		 Пример:
<blockquote><pre>
%!j 4 20      - отформатировать весь текст по ширине в 20 
                символов с отступом из 4х пробелов.
</pre></blockquote>
<p />
<h6><a name="=show=  показать блок в цвете."></a> <code>show</code>  показать блок в цвете. </h6>
Пример:
<blockquote><pre>
%!s           - показать как будет выглядеть весь текст с учетом цветов.
</pre></blockquote>
<p />
<p />
<h5><a name="=(.,.)Y[рег]=   Копировать (yank"></a> <code>(.,.)Y[рег]</code>   Копировать (yank). </h5>
Запомнить в регистре <code>рег</code> содержимое блока, указанного диапазоном адресов. 
Последняя скопированная строка блока становится текущей.
<p />
Пример: Пускай текст содердит:
<blockquote><pre>
ляляляля
{       
бубубубу
бебебебе
}       
ляляляля
ляляляля
</pre></blockquote>
<p />
Выполним:
<blockquote><pre>
: /{/,/}/Y   - запоминаем в регистре поумолчанию все строки 
	       между открывающей и закрывающей фигурной 
	       скобкой, включая строки со скобками.
</pre></blockquote>
<p />
<h5><a name="=(.)P[рег]=     Вставить содержи"></a> <code>(.)P[рег]</code>     Вставить содержимое регистра <code>рег</code> после указанного адреса. </h5>
Последняя вставленная строка становится текущей.
<p />
Пример:
<blockquote><pre>
: $P         - вставить содержимое регистра поумолчанию в
               конец редактируемого текста.
</pre></blockquote>
<p />
<h5><a name="=(.,.)s[парам]= Найти и заменить"></a> <code>(.,.)s[парам]</code> Найти и заменить подстроку в указанном диапазоном адресов блоке. </h5>
Параметр начинается с <em>символа-разделителя</em> и состоит из трех частей, разделенных одним и тем же символом. 
Так <code>s/aa/bb/</code> то же самое что и <code>s%aa%bb%</code>. 
<p />
Первая часть параметра - регулярное выражение (определяет что именно надо заменить).
<p />
Вторая часть - то, на что необходимо заменить текст, 
удовлетворяющий регулярному выражению из первой части.
<p />
Треья часть содержит флаги, состоящие из последовательности символов:
<table cellspacing="0" id="table4" cellpadding="0" class="twikiTable" rules="rows" border="1">
	<tbody>
		<tr class="twikiTableOdd twikiTableRowdataBgSorted0 twikiTableRowdataBg0">
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol0 twikiFirstCol"> p </td>
			<td bgcolor="#ffffff" valign="top" class="twikiTableCol1 twikiLastCol"> печатать каждую замену на экран </td>
		</tr>
		<tr class="twikiTableEven twikiTableRowdataBgSorted1 twikiTableRowdataBg1">
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol0 twikiFirstCol twikiLast"> g </td>
			<td bgcolor="#edf4f9" valign="top" class="twikiTableCol1 twikiLastCol twikiLast"> заменять все вхождения регулярного выражения в строку, а не только первое. </td>
		</tr>
	</tbody></table>
<p />
		 Пример:
<blockquote><pre>
: %s/да/нет/g        -- заменить во всем тексте "да" на "нет"
</pre></blockquote>
<p />
Еще пример: Пускай текст содердит:
<blockquote><pre>
ляляляля
{       
бубубубу
бебебебе
}       
ляляляля
ляляляля
</pre></blockquote>
<p />
Выполним:
<blockquote><pre>
: /{/+1,/}/-1s/^/    /  -- в каждой строке после {, но до }
                           добавить в начало четыре пробела.
</pre></blockquote>
<p />
Текст после выполнения команды:
<blockquote><pre>
ляляляля
{       
    бубубубу
    бебебебе
}       
ляляляля
ляляляля
</pre></blockquote>
<p />
<h5><a name="=u=             Откат."></a> <code>u</code>             Откат. </h5>
Отменяет изменения сделанные предыдущей командой.
Число хранимых изменений не ограничено. То есть, в любой
момент можно вернуться к начальному варианту редактируемого
текста.
<p />
<h5><a name="=q=             Завершить редакт"></a> <code>q</code>             Завершить редактирование. </h5>
<p />
<p />
<h3><a name="Режим построчного ввода"></a> Режим построчного ввода </h3>
В режиме построчного ввода промпт состоит из символа <code>+</code> <em>(плюс)</em>. 
Вводимый текст будет как есть добавлен в редактируемый буфер.
Вернуться в комманднай режим из режима построчного ввода (завершить построчный ввод текста)
можно введя строку состоящую из ровно одно символа <code>.</code> <em>(точка)</em>. 