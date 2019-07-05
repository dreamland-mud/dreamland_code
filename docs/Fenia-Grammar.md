<pre><font color="black"><a name="fenia"></a><a name="феня"></a><a name="grammar"></a><a name="грамматика"></a>
<h3><font color="black">Грамматика языка феня</font></h3>
<i>(оригинал этой книги находится в библиотеке на Улице Вязов)</i>
<br>
Ходят слухи, что язык феня (fenia) всего лишь диалект эльфийского (quenia).
Данный труд призван развеять подобные заблуждения и показать, что ничего
общего с эльфийской тарабарщиной феня не имеет. 

Введение                    - см <a href="#intro">intro</a>
История происхождения языка - см <a href="#background">background</a>
Свод правил                 - см <a href="#syntax">syntax</a>

Про все зеленые слова можно посмотреть на соответствующих страницах.

Базовый вариант 52 страницы. Тираж 1 - 1. 

			    (снизу кривым почерком: и здесь тоже был Filths)
</font>
</p><hr><p>
<font color="black"><a name="история"></a>
<a name="history"></a>
<a name="background"></a>

<h3><font color="black">История</font></h3>

Первозданный вариант фени разрабатывался на чистом ansi-C как язык мобпрог 
для Forgotten Dungeon (киевский пулетех), но так и не был доведен до ума.
В 200?м году рабочий вариант языка был защищен как дипломный проект по теме
"язык моделирования поведения объектов" на мехмате Одесского нац. универа.
Много лет спустя, множество раз переписанный, в конечном итоге был прикручен
Руффиной к многострадальному DreamLandу почти без потери функциональности.
Говорить об отличиях реализации нет смысла, так как общего кода старая и
новая феня не имеет. Тем не менее принципы и синтаксис остались практически
неизменными.
</font>
</p><hr><p>
<font color="black"><a name="введение"></a>
<a name="intro"></a>

<h3><font color="black">Введение</font></h3>

При понимании синтаксических структур фени (см <a href="#syntax">syntax</a>) следует иметь в виду 
цели и задачи, которые стоят перед приложениями, написаными на ней. Основная 
задача - моделирование поведения. Будем понимать под поведением набор правил, 
определяющих реакцию системы на внешние события. Потому язык является
событийно-ориентированым. Это существенный момент для понимания полиморфизма
в фене. Если в строго/слабо типизированных объектно-ориентированных языках 
полиморфизм достигается на уровне протитипов экземпляров (классов данных),
в фене он достигается на уровне самих экземпляров. (см. <a href="#example%20polymorph">example polymorph</a>)
То есть, поведение конкретного экземпляра объекта в фене может существенно
отличаться от его прототипа. Более того, цикл жизни одного и того же объекта
предполагает различные поведения в различные моменты времени, что реализует
множество состояний одного и того же объекта. Удобней всего реализовать 
подобные требования объединив данные и код воедино. Иными словами, код 
представляется некоторым типом данных, с которым можно производить операции,
подобные тем, что производятся с данными.

Феня создавалась как нетипизированный язык. То есть, переменные сами по себе
не имеют типа (в отличии от хранимых в них данных). Такое решение было принято
для того, что бы как можно сильней облегчить процесс обучения и написания 
простых приложений (однако это затрудняет отладку и требует большей 
внимательности, чем в типизированных языках).
</font>
</p><hr><p>
<font color="black"><a name="syntax"></a>
<a name="синтаксис"></a>

<h3><font color="black">Синтаксис</font></h3>

<a href="#char">char</a> = 0..255
<a href="#alpha">alpha</a> = </font><font color="darkmagenta"><b>'a'</b></font><font color="black">..</font><font color="darkmagenta"><b>'z'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'A'</b></font><font color="black">..</font><font color="darkmagenta"><b>'Z'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'а'</b></font><font color="black">..</font><font color="darkmagenta"><b>'я'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'А'</b></font><font color="black">..</font><font color="darkmagenta"><b>'Я'</b></font><font color="black">
<a href="#digit">digit</a> = </font><font color="darkmagenta"><b>'0'</b></font><font color="black">..</font><font color="darkmagenta"><b>'9'</b></font><font color="black">

<a href="#whatever">whatever</a> = char [whatever]
<a href="#strings">strings</a> = string [strings]
string = </font><font color="darkmagenta"><b>'"'</b></font><font color="black"> whatever </font><font color="darkmagenta"><b>'"'</b></font><font color="black"> | </font><font color="darkmagenta"><b>"'"</b></font><font color="black"> whatever </font><font color="darkmagenta"><b>"'"</b></font><font color="black">
<a href="#comment">comment</a> = </font><font color="darkmagenta"><b>'//'</b></font><font color="black"> whatever </font><font color="darkmagenta"><b>'\n'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'/*'</b></font><font color="black"> whatever </font><font color="darkmagenta"><b>'*/'</b></font><font color="black">
<a href="#number">number</a> = digit [number]
numalpha = digit | alpha
<a href="#numalphas">numalphas</a> = numalpha [numalphas]
id = alpha [numalphas]

</font><font color="black">varlist</font><font color="black"> = id [ </font><font color="darkmagenta"><b>','</b></font><font color="black"> varlist ]
</font><font color="black">explist</font><font color="black"> = expr [ </font><font color="darkmagenta"><b>','</b></font><font color="black"> explist ]
stmts = stmt [stmts]

<a href="#function">function</a> = </font><font color="darkmagenta"><b>'function'</b></font><font color="black"> [number] [ </font><font color="darkmagenta"><b>'('</b></font><font color="black"> [varlist] </font><font color="darkmagenta"><b>')'</b></font><font color="black"> </font><font color="darkmagenta"><b>'{'</b></font><font color="black"> [stmts] </font><font color="darkmagenta"><b>'}'</b></font><font color="black"> ]
<a href="#const_exp">const_exp</a> = </font><font color="darkmagenta"><b>'null'</b></font><font color="black"> | number | strings | function

unop = </font><font color="darkmagenta"><b>'!'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'~'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'-'</b></font><font color="black">
<a href="#unop_exp">unop_exp</a> = unop expr

<a href="#mixed_binop">mixed_binop</a> = </font><font color="darkmagenta"><b>'+'</b></font><font color="black">
<a href="#arith_binop">arith_binop</a> = </font><font color="darkmagenta"><b>'*'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'/'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'%'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'-'</b></font><font color="black">
<a href="#bit_binop">bit_binop</a> = </font><font color="darkmagenta"><b>'^'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'&'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'|'</b></font><font color="black">
<a href="#pred_binop">pred_binop</a> = </font><font color="darkmagenta"><b>'<'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'>'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'<='</b></font><font color="black"> | </font><font color="darkmagenta"><b>'>='</b></font><font color="black"> | </font><font color="darkmagenta"><b>'=='</b></font><font color="black"> | </font><font color="darkmagenta"><b>'!='</b></font><font color="black"> 
<a href="#log_binop">log_binop</a> = </font><font color="darkmagenta"><b>'&&'</b></font><font color="black"> | </font><font color="darkmagenta"><b>'||'</b></font><font color="black">
binop = arith_binop | bit_binop | pred_binop | log_binop
<a href="#binop_exp">binop_exp</a> = expr binop expr

<a href="#assign_exp">assign_exp</a> = ref </font><font color="darkmagenta"><b>'='</b></font><font color="black"> expr

<a href="#default_ref">default_ref</a> = id
<a href="#field_ref">field_ref</a> = expr </font><font color="darkmagenta"><b>'.'</b></font><font color="black"> id
<a href="#array_ref">array_ref</a> = expr </font><font color="darkmagenta"><b>'['</b></font><font color="black"> expr </font><font color="darkmagenta"><b>']'</b></font><font color="black">
<a href="#root_ref">root_ref</a> = </font><font color="darkmagenta"><b>'.'</b></font><font color="black"> id
<a href="#ref">ref</a> = default_ref | field_ref | array_ref | root_ref

<a href="#call_exp">call_exp</a> = ref </font><font color="darkmagenta"><b>'('</b></font><font color="black"> [explist] </font><font color="darkmagenta"><b>')'</b></font><font color="black">
<a href="#deref_exp">deref_exp</a> = ref
<a href="#expr">expr</a> = const_exp | unop_exp | binop_exp | call_exp | deref_exp | assign_exp

<a href="#nop_stmt">nop_stmt</a> = </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#comp_stmt">comp_stmt</a> = </font><font color="darkmagenta"><b>'{'</b></font><font color="black"> [stmts] </font><font color="darkmagenta"><b>'}'</b></font><font color="black">
<a href="#if_stmt">if_stmt</a> = </font><font color="darkmagenta"><b>'if'</b></font><font color="black"> </font><font color="darkmagenta"><b>'('</b></font><font color="black"> expr </font><font color="darkmagenta"><b>')'</b></font><font color="black"> stmt [ </font><font color="darkmagenta"><b>'else'</b></font><font color="black"> stmt ]
<a href="#for_stmt">for_stmt</a> = </font><font color="darkmagenta"><b>'for'</b></font><font color="black"> </font><font color="darkmagenta"><b>'('</b></font><font color="black"> [explist] </font><font color="darkmagenta"><b>';'</b></font><font color="black"> [expr] </font><font color="darkmagenta"><b>';'</b></font><font color="black"> [explist] </font><font color="darkmagenta"><b>')'</b></font><font color="black"> stmt
<a href="#break_stmt">break_stmt</a> = </font><font color="darkmagenta"><b>'break'</b></font><font color="black"> </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#continue_stmt">continue_stmt</a> = </font><font color="darkmagenta"><b>'continue'</b></font><font color="black"> </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#exp_stmt">exp_stmt</a> = expr </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#return_stmt">return_stmt</a> = </font><font color="darkmagenta"><b>'return'</b></font><font color="black"> [expr] </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#throw_stmt">throw_stmt</a> = </font><font color="darkmagenta"><b>'throw'</b></font><font color="black"> [expr] </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#var_stmt">var_stmt</a> = </font><font color="darkmagenta"><b>'var'</b></font><font color="black"> varlist </font><font color="darkmagenta"><b>';'</b></font><font color="black">
<a href="#try_stmt">try_stmt</a> = </font><font color="darkmagenta"><b>'try'</b></font><font color="black"> stmt </font><font color="darkmagenta"><b>'catch'</b></font><font color="black"> </font><font color="darkmagenta"><b>'('</b></font><font color="black"> id </font><font color="darkmagenta"><b>')'</b></font><font color="black"> stmt
<a href="#stmt">stmt</a> = nop_stmt | comp_stmt | if_stmt | for_stmt | break_stmt | continue_stmt
     | exp_stmt | return_stmt | throw_stmt | var_stmt | try_stmt
</font>
</p><hr><p>
<font color="black"><a name="example try catch"></a>

<h3><font color="black">Пример: (example try catch)</font></h3>

Пускай метод lookup() реализован как в <a href="#example%20throw">example throw</a>.
    var o, default_o;
    ...
    try {
	o = lookup("бебе");
	/*
	 * если управление дошло до этого месте - lookup завершился
	 * успешно (без исключения).
	 */
    } catch(e) {
	/*
	 * во время выполнения lookup() произощло исключение e
	 */
	if(e == "не найдено")
	    o = default_o;  // это исключение было кинуто явно lookupом
	else
	    throw e;	    // неизвестное исключение. кидаем дальше,
			    // может кто-то поймает
    }
    ...


</font>
</p><hr><p>
<font color="black"><a name="example return 2"></a>
<a name="example throw"></a>

<h3><font color="black">Пример: (example return 2, example throw)</font></h3>

lookup = function (n) {
    var i;
    
    for(i = .list_head; i != null; i = i.next) {
	if(i.name == n)
	    return i;	// нашли i. завершить выполнение ф-и и вернуть i.
    }
    /*
     * если управление дошло до этого места - ничего не найдено.
     * генерируем исключение.
     */
     throw "не найдено";
    /*
     * до этого места управление не доходит.
     * бессмысленно что-то писать после throw.
     */
}

</font>
</p><hr><p>
<font color="black"><a name="example continue"></a>

<h3><font color="black">Пример: (example continue)</font></h3>

process = function () {
    var i;

    for(i = .list_head; i != null; i = i.next) {
	/* если i не нуждается в обработке - сразу перейти к следующему i */
	if(!i.need_process)
	    continue;

	/* если i уже обработано ранее - сразу перейти к следующему i */
	if(i.processed)
	    continue;

	/*
	 * следующее утверждение выполняется только для объектов, которые 
	 * нуждаются в обработке (i.need_process - 'истина') и для тех, 
	 * что еще не были обработаны ранее (i.precessed == 'ложь')
	 */
	i.process();
    }
}

</font>
</p><hr><p>
<font color="black"><a name="example break"></a>
<a name="example return 1"></a>

<h3><font color="black">Пример: (example break, example return 1)</font></h3>

lookup = function (n) {
    var i;
    
    for(i = .list_head; i != null; i = i.next) {
	if(i.name == n)
	    break;
    }
    /*
     * i будет равно null, если в списке .list_head не оказалось объекта
     * с полем name равным параметру n.
     * иначе i будет содержать ссылку на объект содержащий поле name равным
     * параметру n.
     *
     * завершаем ф-ю с возвращаемым значением i.
     */
     return i;
}

</font>
</p><hr><p>
<font color="black"><a name="example mixed_binop"></a>

<h3><font color="black">Пример: (example mixed_binop)</font></h3>

правая часть приводится к типу левой:    
    "2" + 2 // - строка '22'
но
    2 + "2" // - число '4' 
    
</font>
</p><hr><p>
<font color="black"><a name="example polymorph"></a>

<h3><font color="black">Пример: (example polymorph)</font></h3>

polyInit = function() {
    /*конструктор абстрактной фигуры*/
    .types.Figure = function() {
	this = .Map();

	/*инициализация общих полей и методов*/
	...
	
	/*реализация подсчета площади поумолчанию*/
	s = function() {
	    throw "не умею считать площадь для этой фигуры";
	};
	
	return this;
    };

    .types.Rect = function(x, y) {
	this = .types.Figure();

	width = x;
	height = y;
	
	/*реализация подсчета площади прямоугольника*/
	s = function() {
	    return width*height;
	};
	
	return this;
    };
    
    .types.Circle = function(r) {
	this = .types.Figure();

	radius = r;
	
	/*реализация подсчета площади круга*/
	s = function() {
	    return 314*r*r/100;
	};
	
	return this;
    };
}

</font>
</p><hr><p>
<font color="black"><a name="example scope 2"></a>

<h3><font color="black">Пример: (example scope 2)</font></h3>

   function () {
       var a, t;
       
       a = 5;
       
       t = function() {
	   .print(a);	  // переменная 'a' не видна из вложенной функции.
       };

       t();
   }

</font>
</p><hr><p>
<font color="black"><a name="example scope 1"></a>

<h3><font color="black">Пример: (example scope 1)</font></h3>

   ...
   var a;
   a = 2;
   {
       var a;
       a = 1;
       .print(a);   // напечатает 1;
   }
   .print(a);	    // напечатает 2;
   ...
   Т.к. составное утверждение создает новую область видимости, переменная a,
   объявленная внутри фигурных скобок, не будет видна снаружи.

</font>
</p><hr><p>
<font color="black"><a name="root object"></a>
<a name="корневой объект"></a>

<h3><font color="black">Корневой объект</font></h3>

Контейнер, содержащий глобальную информацию, доступную всем потокам управления
и всем функциям через <a href="#root_ref">root_ref</a>.

</font>
</p><hr><p>
<font color="black"><a name="область видимости"></a>
<a name="scope"></a>

<h3><font color="black">Область видимости переменных</font></h3>

Область видимости - контейнер содержащий локальные переменные.

Кроме переменных область видимости содержит указатель на обрамляющую
область видимости. 

Область видимости с нулевым указателем на предыдущую область видимости
называется корневой.

Каждый поток управления фени имеет указатель на свою текущую область 
видимости. 

Создавая новую не корневую область видимости, ее указатель на обрамляющую
область инициализируется текущей областью видимости этого потока, 
сам указатель но текущую область видимости устанавливается на новосозданную
область. Разрушить область видимости можно только в случае, если она текущая.
При этом текущая область видимости должна будет указывать на обрамляющую 
область видимости разрушаемой.

Говоря, что утверждение создает новую область видимости понимается то, что
вышеизложенным способом создается временная область видимости, которая 
существует пока выполняется это утверждение, после чего она разрушается.
(см <a href="#example%20scope%201">example scope 1</a>)
  
  
Таким образом, множество доступных локальных переменных определяется
объединением переменных созданных во всех областях винимости начиная с текущей
и заканчивая корневой. При чем переменные объявленные во внутренних областях
видимости перекрывают переменные с тем же именем во внешних областях.

Корневая область видимости создается функцией перед началом выполнения.
Таки образом переменные объявленные в вызывающей функции недоступны для
вызываемой. Даже если вызываемая функция объявлена в вызывающей.
(см <a href="#example%20scope%202">example scope 2</a>)

</font>
</p><hr><p>
<font color="black"><a name="генерировать исключение"></a>
<a name="throw_stmt"></a>

<h3><font color="black">Генерировать исключение</font></h3>

Syntax: </font><font color="black"><b>throw_stmt = 'throw' [expr] ';'</b></font><font color="black">

Если есть <a href="#expr">expr</a>, вычислить его значение и привести к строке.
Эта строка будет использована как текстовое представление 
генерируемого исключения.
Перейти к пункту 3. самого внутреннего <a href="#try_stmt">try_stmt</a>.

См. также: <a href="#example%20throw">example throw</a>

</font>
</p><hr><p>
<font color="black"><a name="поймать исключение"></a>
<a name="try_stmt"></a>

<h3><font color="black">Поймать исключение</font></h3>

Syntax: </font><font color="black"><b>try_stmt = 'try' stmt1 'catch' '(' id ')' stmt2</b></font><font color="black">

0. Создать новую вложенную область видимости переменных. (см <a href="#scope">scope</a>)
1. пытается выполнить stmt1. 
2. конец выполнения этого утверждения

следующие действия выполняются если произошло исключение:
3. создать новую область видимости переменных, (см <a href="#scope">scope</a>)
4. добавить в нее переменную с именем id,
5. присвоить ей строковое описание исключения
6. выполнить stmt2

См. также: <a href="#example%20try%20catch">example try catch</a>, <a href="#stmt">stmt</a>

</font>
</p><hr><p>
<font color="black"><a name="var_stmt"></a>
<a name="декларирование"></a>

<h3><font color="black">Декларировать локальные переменные</font></h3>

Syntax: </font><font color="black"><b>varlist = id [ ',' varlist ]</b></font><font color="black">
Syntax: </font><font color="black"><b>var_stmt = 'var' varlist ';'</b></font><font color="black">

В самой внутренней области видимости (см. <a href="#scope">scope</a>) добавляет переменные
с именами (id) из varlist. Новосозданные переменные имеют значения типа NONE.

</font>
</p><hr><p>
<font color="black"><a name="завершить функцию"></a>
<a name="return_stmt"></a>

<h3><font color="black">Завершить выполнение функции</font></h3>

Syntax: </font><font color="black"><b>return_stmt = 'return' [expr] ';'</b></font><font color="black">

Если <a href="#expr">expr</a> опущено - результат вычисления функции имеет тип NONE.
Иначе - результат вычисления <a href="#expr">expr</a>.
См. также: <a href="#example%20return%201">example return 1</a>, <a href="#example%20return%202">example return 2</a>

</font>
</p><hr><p>
<font color="black"><a name="выполнить выражение"></a>
<a name="exp_stmt"></a>

<h3><font color="black">Выполнить выражение</font></h3>

Syntax: </font><font color="black"><b>exp_stmt = expr ';'</b></font><font color="black">

Вычислить выражение <a href="#expr">expr</a>, проигнорировав результат вычисления.

</font>
</p><hr><p>
<font color="black"><a name="continue_stmt"></a>

<h3><font color="black">Перейти к следующей итерации</font></h3>

Syntax: </font><font color="black"><b>continue_stmt = 'continue' ';'</b></font><font color="black">

После выполнения этого утверждения, управление передается пункту 5
самого внутреннего цикла выполняемой функции. Если continue используется
вне цикла - генерируется исключение.
(см <a href="#for_stmt">for_stmt</a>, <a href="#example%20continue">example continue</a>)

</font>
</p><hr><p>
<font color="black"><a name="прервать цикл"></a>
<a name="break_stmt"></a>

<h3><font color="black">Прервать цикл</font></h3>

Syntax: </font><font color="black"><b>break_stmt = 'break' ';'</b></font><font color="black">

Прервать выполнение цикла. После выполнения этого утверждения,
управление передается пункту 7 самого внутреннего цикла выполняемой 
функции. Если break используется вне цикла - генерируется исключение.
См. также: <a href="#for_stmt">for_stmt</a>, <a href="#example%20break">example break</a>

</font>
</p><hr><p>
<font color="black"><a name="for_stmt"></a>
<a name="цикл"></a>

<h3><font color="black">Цикл</font></h3>

Syntax: </font><font color="black"><b>for_stmt = 'for' '(' [explist1] ';' [expr] ';' [explist2] ')' stmt</b></font><font color="black">


0. Создать новую вложенную область видимости переменных. (см <a href="#scope">scope</a>)
1. Вычислить выражения из explist1, игнорируя результаты.
2. Вычислить <a href="#expr">expr</a> и привести результат к лог.значению.
3. Если результат 'ложь' - перейти к 7.
4. Выполнить <a href="#stmt">stmt</a>.
5. Вычислить выражения из explist2, игнорируя результаты.
6. Перейти к 2.
7. конец обработки этого утверждения.

Принятые названия:
explist1 - инициализация цикла,
<a href="#expr">expr</a>      - условие выхода из цикла,
explist2 - шаг цикла,
<a href="#stmt">stmt</a>     - тело цикла.

См. также: <a href="#break_stmt">break_stmt</a>, <a href="#continue_stmt">continue_stmt</a>
</font>
</p><hr><p>
<font color="black"><a name="if_stmt"></a>
<a name="условие"></a>

<h3><font color="black">Условие</font></h3>

Syntax: </font><font color="black"><b>if_stmt = 'if' '(' expr ')' stmt1 [ 'else' stmt2 ]</b></font><font color="black">

Создает новую вложенную область видимости переменных. (см <a href="#scope">scope</a>)
Вычисляет значение <a href="#expr">expr</a> и приводит (см <a href="#casting">casting</a>) его к лог.значению.
Если результат 'истина' - выполнить stmt1, иначе stmt2 (если есть).

</font>
</p><hr><p>
<font color="black"><a name="составное утверждение"></a>
<a name="comp_stmt"></a>

<h3><font color="black">Составное утверждение</font></h3>

Syntax: </font><font color="black"><b>stmts = stmt [stmts]</b></font><font color="black">
Syntax: </font><font color="black"><b>comp_stmt = '{' [stmts] '}'</b></font><font color="black">

Выполнить все утверждения (<a href="#statement">statement</a>) из stmts. Или ничего, если stmts 
опущено. Создает новую вложенную область видимости переменных. (см <a href="#scope">scope</a>)

</font>
</p><hr><p>
<font color="black"><a name="пустое утверждение"></a>
<a name="nop_stmt"></a>

<h3><font color="black">Пустое утверждение</font></h3>

Syntax: </font><font color="black"><b>nop_stmt = ';'</b></font><font color="black">

Ничего не делать.

</font>
</p><hr><p>
<font color="black"><a name="stmt"></a>
<a name="statement"></a>
<a name="утверждение"></a>

<h3><font color="black">Утверждение</font></h3>

Syntax: </font><font color="black"><b>stmt = nop_stmt | comp_stmt | if_stmt | for_stmt | break_stmt 
             | continue_stmt | exp_stmt | return_stmt | throw_stmt 
	     | var_stmt | try_stmt</b></font><font color="black">

Синтаксическая сруктура, семантическое значение которой позволяет выполнить
то или иное действие (императив).
См. также: <a href="#nop_stmt">nop_stmt</a>, <a href="#comp_stmt">comp_stmt</a>, <a href="#if_stmt">if_stmt</a>, <a href="#for_stmt">for_stmt</a>, <a href="#break_stmt">break_stmt</a>,
           <a href="#continue_stmt">continue_stmt</a>, <a href="#exp_stmt">exp_stmt</a>, <a href="#return_stmt">return_stmt</a>, <a href="#throw_stmt">throw_stmt</a>,
	   <a href="#var_stmt">var_stmt</a>, <a href="#try_stmt">try_stmt</a>

</font>
</p><hr><p>
<font color="black"><a name="базовые обозначения"></a>
<a name="base_def"></a>
<a name="char"></a>
<a name="alpha"></a>
<a name="digit"></a>
<a name="whatever"></a>

<h3><font color="black">Базовые обозначения</font></h3>

Syntax: </font><font color="black"><b>char = 0..255</b></font><font color="black">
Произвольный символ.

Syntax: </font><font color="black"><b>alpha = 'a'..'z' | 'A'..'Z' | 'а'..'я' | 'А'..'Я'</b></font><font color="black">
Буква.

Syntax: </font><font color="black"><b>digit = '0'..'9'</b></font><font color="black">
Цифра.

Syntax: </font><font color="black"><b>whatever = char [whatever]</b></font><font color="black">
Последовательность произвольных символов любой длины.

</font>
</p><hr><p>
<font color="black"><a name="number"></a>
<a name="число"></a>

<h3><font color="black">Число</font></h3>

Syntax: </font><font color="black"><b>number = digit [number]</b></font><font color="black">

Константа NUMBER.
Символическая запись числа в десятичном представлении.
См. также: <a href="#digit">digit</a>

</font>
</p><hr><p>
<font color="black"><a name="strings"></a>
<a name="строка"></a>

<h3><font color="black">Строка</font></h3>

Syntax: </font><font color="black"><b>strings = string [strings]</b></font><font color="black">
Syntax: </font><font color="black"><b>string = '"' whatever '"' | "'" whatever "'"</b></font><font color="black">

Константа типа STRING.
\n	  - символ перевоа строки
\r	  - символ возврата каретки
\t	  - табуляция
\ + любой другой символ, например кавычка, заменяется на себя же без '\'
См. также: <a href="#whatever">whatever</a>
</font>
</p><hr><p>
<font color="black"><a name="comment"></a>
<a name="коментарий"></a>

<h3><font color="black">Коментарий</font></h3>

Syntax: </font><font color="black"><b>comment = '//' whatever '\n' | '/*' whatever '*/'</b></font><font color="black">

Коментарии не являются частью дерева разбора и игнорируются.
См. также: <a href="#whatever">whatever</a>
</font>
</p><hr><p>
<font color="black"><a name="identifier"></a>
<a name="идентификатор"></a>
<a name="numalphas"></a>

<h3><font color="black">Идентификатор</font></h3>

Syntax: </font><font color="black"><b>numalpha = digit | alpha</b></font><font color="black">
Syntax: </font><font color="black"><b>numalphas = numalpha [numalphas]</b></font><font color="black">
Syntax: </font><font color="black"><b>id = alpha [numalphas]</b></font><font color="black">

Последовательность букв и цифр, начинающаяся с буквы (см. <a href="#digit">digit</a>, <a href="#alpha">alpha</a>).
Используется в именах полей и переменных.

Каждый идентификатор регистрируется в двух ассоциативных массивах.
Один из них ставит последовательности букв и цифр в соответствие 
порядковый номер идентификатора, другой - наоборот, номеру ставит
в соответствие строку. За время жизни программы эти массивы только растут.
Все операции над идентификаторами производятся над их порядковым номером.
</font>
</p><hr><p>
<font color="black"><a name="function"></a>
<a name="функция"></a>

<h3><font color="black">Функция</font></h3>

Syntax: </font><font color="black"><b>varlist = id [ ',' varlist ]</b></font><font color="black">
Syntax: </font><font color="black"><b>stmts = stmt [stmts]</b></font><font color="black">
Syntax: </font><font color="black"><b>function = 'function' [number] [ '(' [varlist] ')' '{' [stmts] '}' ]</b></font><font color="black">

Константа, определяющая некоторую последовательность действий.

Необязательный параметр number - идентификатор функции, при помощи которого
всегда можно изменить логику функции. Если идентификатор указан, остальную
часть синтаксической структуры можно опустить - константа будет ссылаться на
объявленую ранее с тем же идентификатором функцию. Если идентификатор не 
указан, выбирается следующий свободный.

stmts - последовательность императивных утверждений (<a href="#stmt">stmt</a>), составляющих логику 
функции.

При вызове создает корневую область видимости переменных. (см <a href="#scope">scope</a>)
В созданную область видимости добавляет переменную с именем 'this' и 
инициализирует ее соответствующим параметром. В ту же область добавляет
переменные с именами из varlist, инициализируя их значения соответствующими
значениями выбранными из списка передаваемых значений. Число переменных в 
varlist должно в точности совпадать с числом выражений в explist 
соответствующего <a href="#call_exp">call_exp</a>. Последовательно выполняет утверждения из stmts.
После выполнения последнего утверждения, возвращает значение типа NONE.
</font>
</p><hr><p>
<font color="black"><a name="const_exp"></a>
<a name="константа"></a>

<h3><font color="black">Константа</font></h3>

Syntax: </font><font color="black"><b>const_exp = 'null' | number | strings | function</b></font><font color="black">

Семантический узел такого типа хранит готовый регистр, представляющий собой
данные, не меняющиеся в ходе выполнения программы. Результатом выполнения
такого выражения служит хранимый регистр.  null - константа типа NONE. 
См. также: <a href="#number">number</a>, <a href="#strings">strings</a>, <a href="#function">function</a>, <a href="#register">register</a>
</font>
</p><hr><p>
<font color="black"><a name="корневая ссылка"></a>
<a name="root_ref"></a>

<h3><font color="black">Ссылка на поле в корневом объекте</font></h3>

Syntax: </font><font color="black"><b>root_ref = '.' id</b></font><font color="black">

Ссылка указывает на поле с именем id внутри контейнера root. 
(см. <a href="#root%20object">root object</a>)
</font>
</p><hr><p>
<font color="black"><a name="ссылка на элемент массива"></a>
<a name="array_ref"></a>

<h3><font color="black">Ссылка на элемент массива</font></h3>

Syntax: </font><font color="black"><b>array_ref = exp1 '[' exp2 ']'</b></font><font color="black">

Результат вычисления выражения exp2 является ключом внутри контейнера,
получаемого в результате вычисления exp1.
</font>
</p><hr><p>
<font color="black"><a name="ссылка на поле"></a>
<a name="field_ref"></a>

<h3><font color="black">Ссылка на поле</font></h3>

Syntax: </font><font color="black"><b>field_ref = expr '.' id</b></font><font color="black">

Ссылка указывает на поле с именем id в контейнере, получаемом в результате 
вычисления значения <a href="#expr">expr</a>.
</font>
</p><hr><p>
<font color="black"><a name="ссылка по умолчанию"></a>
<a name="default_ref"></a>

<h3><font color="black">Ссылка по умолчанию</font></h3>

Syntax: </font><font color="black"><b>default_ref = id</b></font><font color="black">

Данная ссылка указывает на локальную переменную с именем id в текущей
облати видимости переменных (см. <a href="#scope">scope</a>). В случае если такой переменной нет,
ссылка указывает на поле с именем id внутри контейнера this.
</font>
</p><hr><p>
<font color="black"><a name="ref"></a>
<a name="ссылка"></a>

<h3><font color="black">Ссылка на хранилище данных</font></h3>

Syntax: </font><font color="black"><b>ref = default_ref | field_ref | array_ref | root_ref</b></font><font color="black">

Семантическое значение ссылки остоит из двух частей: контейнера и ключа для 
доступа к регистру внутри контейнера. Типичный пример ссылки - пара 
'массив' - 'индекс'. 

Ссылка реализует три метода метода доступа к адресуемому регистру:
 * присваивание  
   установить значение адресуемого регистра
 * разыменование 
   получить значение адресуемого регистра
 * вызов метода  
   адресуемый регистр является функцией. Данный метод доступа вызывает 
   эту ф-ю и возвращает значение ввиде регистра.

См. также: <a href="#default_ref">default_ref</a>, <a href="#field_ref">field_ref</a>, <a href="#array_ref">array_ref</a>, <a href="#root_ref">root_ref</a>
</font>
</p><hr><p>
<font color="black"><a name="expr"></a>
<a name="выражение"></a>
<a name="register"></a>
<a name="регистр"></a>

<h3><font color="black">Выражение</font></h3>

Syntax: </font><font color="black"><b>expr = const_exp | unop_exp | binop_exp 
            | call_exp | deref_exp | assign_exp </b></font><font color="black">


Синтаксическая структура, семантическое значение которой позволяет вычислить
некоторое значение. Хранилище таких значений называется регистром.
Значения регистров бывают следующих типов:
 * NONE	    - нет значения, неопределено.
 * STRING   - строка символов
 * NUMBER   - целое знаковое 32битовое число,  служит так же как 
              лог.значение (если равно нулю - ложь, иначе - истина).
 * OBJECT   - сложный объект (содержит ссылки на другие объекты)
 * FUNCTION - ф-ция
 * IDENTIFIER - испотльзуется внутрене. хранит число, являющееся
                семантическим значением лексемы ID.
Все базовые операции с данными в фене производятся над регистрами.
Существует неявная система приведений одного типа к другому (см <a href="#casting">casting</a>).
См. также: <a href="#const_exp">const_exp</a>, <a href="#unop_exp">unop_exp</a>, <a href="#binop_exp">binop_exp</a>, <a href="#call_exp">call_exp</a>, 
           <a href="#deref_exp">deref_exp</a>, <a href="#assign_exp">assign_exp</a>
</font>
</p><hr><p>
<font color="black"><a name="deref_exp"></a>
<a name="разыменование"></a>

<h3><font color="black">Разыменование</font></h3>

Syntax: </font><font color="black"><b>deref_exp = ref</b></font><font color="black">

Вызывает разыменование ссылки (получение значения). (см <a href="#ref">ref</a>)
</font>
</p><hr><p>
<font color="black"><a name="операции ветвления"></a>
<a name="log_binop"></a>

<h3><font color="black">Логические операции (операции ветвления)</font></h3>

Syntax: </font><font color="black"><b>log_binop = '&&' | '||'</b></font><font color="black">
Syntax: </font><font color="black"><b>expr log_binop expr</b></font><font color="black">

Особенностью этих операций является то, что в определенных ситуациях значение
правой части не вычисляется.

см. <a href="#logical%20and">logical and</a> и <a href="#logical%20or">logical or</a>, <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="бинарные предикаты"></a>
<a name="pred_binop"></a>

<h3><font color="black">Бинарные предикаты</font></h3>

Syntax: </font><font color="black"><b>pred_binop = '<' | '>' | '<=' | '>=' | '==' | '!=' </b></font><font color="black">
Syntax: </font><font color="black"><b>expr pred_binop expr</b></font><font color="black">

Сначала вычисляется левая и правая часть (см.<a href="#expr">expr</a>).
Все бинарные предикаты определены для следующих типов левой части: NUMBER, 
IDENTIFIER, STRING. Правая часть приводится (см <a href="#casting">casting</a>) к типу левой.
Значения типа NUMBER сравниваются как знаковые числа, IDENTIFIER - по 
порядковому номеру регистрации идентификатора, STRING - лексикографически.
Кроме того, операторы '==' и '!=' определены для типов OBJECT, FUNCTION и NONE.
В случае если значение левой, или правой части имеет тип NONE - сравнивается
не содержимле, а тип значений. Таким образом, 'null == x' и 'x == null' - 
истина тогда и только тогда, когда 'x' имеет тип NONE; так как приведение
типов не вызывается никаких иключений возникнуть не может. 
В случае если тип значения левой, и правой части отличен от NONE, значение 
правой части приводится к типу левой. Значения типов OBJECT и FUNCTION 
сравниваются как указатели.
</font>
</p><hr><p>
<font color="black"><a name="побитовые операции"></a>
<a name="bit_binop"></a>

<h3><font color="black">Побитовые операции</font></h3>

Syntax: </font><font color="black"><b>bit_binop = '^' | '&' | '|'</b></font><font color="black">
Syntax: </font><font color="black"><b>expr bit_binop expr</b></font><font color="black">

Вычисляется значение левой и правой части (см. <a href="#expr">expr</a>).
В случае, если тип вычесленного значения левой части не является NUMBER, 
генерируется NotImplementedException. Значение правой части приводится 
(см <a href="#casting">casting</a>) к числу. Выполняется необходимая побитовая операция:
 - '^' - исключающее или
 - '&' - и
 - '|' - или

Note:
Для i = 0..31:
Бит i в числе 'l & r' будет равен единице <=> биты i в l и r равны единице.
Бит i в числе 'l | r' будет равен нулю <=> биты i в l и r равны нулю.
Бит i в числе 'l ^ r' будет равен нулю <=> биты i в l и r равны между собой.

<=> - тогда и только тогда.
</font>
</p><hr><p>
<font color="black"><a name="смешанные операции"></a>
<a name="mixed_binop"></a>

<h3><font color="black">Смешанные операции</font></h3>

Syntax: </font><font color="black"><b>mixed_binop = '+'</b></font><font color="black">
Syntax: </font><font color="black"><b>expr mixed_binop expr</b></font><font color="black">

Вычисляется левая и правая часть. 
Далее действие зависит от типа значения левой части:

 * NUMBER - значение правой части приводится к числу.
   Результат NUMBER = арифметическому сложению значений левой и правой части. 
                      (см. <a href="#arith_binop">arith_binop</a>)
 * STRING - значение правой части приводится к строке.
   Результат STRING = катанация (склейка) значений левой и правой части.
 * для всех остальных типов значения левой части генерируется
   NotImplementedException

См. также: <a href="#casting">casting</a>, <a href="#example%20mixed_binop">example mixed_binop</a>, <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="арифметические операции"></a>
<a name="arith_binop"></a>

<h3><font color="black">Арифметические операции</font></h3>

Syntax: </font><font color="black"><b>arith_binop = '*' | '/' | '%' | '-'</b></font><font color="black">
Syntax: </font><font color="black"><b>expr arith_binop expr</b></font><font color="black">

Вычисляется значение левой и правой части.
В случае, если тип вычесленного значения левой части не является NUMBER, 
генерируется NotImplementedException. Значение правой части приводится 
(см <a href="#casting">casting</a>) к числу. Выполняется необходимая целочисленная арифметическая
операция:
 - '/' - деление
 - '%' - остаток от деления
 - '*' - умножение
 - '-' - вычитание

Note:
если
    N = A / Z - целочисленное деление
    R = A % Z - остаток от целочисленного деления
то
    A = N*Z + R

См. также: <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="бинарные операции"></a>
<a name="binop_exp"></a>

<h3><font color="black">Бинарные операции</font></h3>

Syntax: </font><font color="black"><b>mixed_binop = '+'</b></font><font color="black">
Syntax: </font><font color="black"><b>arith_binop = '*' | '/' | '%' | '-'</b></font><font color="black">
Syntax: </font><font color="black"><b>bit_binop = '^' | '&' | '|'</b></font><font color="black">
Syntax: </font><font color="black"><b>pred_binop = '<' | '>' | '<=' | '>=' | '==' | '!=' </b></font><font color="black">
Syntax: </font><font color="black"><b>log_binop = '&&' | '||'</b></font><font color="black">
Syntax: </font><font color="black"><b>binop = arith_binop | bit_binop | pred_binop | log_binop</b></font><font color="black">
Syntax: </font><font color="black"><b>binop_exp = expr binop expr</b></font><font color="black">

<a href="#mixed_binop">mixed_binop</a> - действие операции зависит от типа левой части
<a href="#arith_binop">arith_binop</a> - арифметические операции, определены только для чисел
<a href="#bit_binop">bit_binop</a>   - побитовые операции, определены только для чисел
<a href="#pred_binop">pred_binop</a>  - предикаты (результат - число 1 (истина), или 0 (ложь))
<a href="#log_binop">log_binop</a>   - операции ветвления  (см. <a href="#logical%20and">logical and</a> и <a href="#logical%20or">logical or</a>)
См. также: <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="унарные операции"></a>
<a name="unop_exp"></a>

<h3><font color="black">Унарные операции</font></h3>

Syntax: </font><font color="black"><b>unop = '!' | '~' | '-'</b></font><font color="black">
Syntax: </font><font color="black"><b>unop_exp = unop expr</b></font><font color="black">

Вычисляет значение <a href="#expr">expr</a>.

 * '!' - значение приводится к лог.выражению. 
   Результат NUMBER = 1, если результат приведения - ложно
   Результат NUMBER = 0, если результат приведения - истина
 
 * '~' - аргумент приводится к числу.
   Результат NUMBER = побитовому отрицанию результата приведения

 * '-' - аргумент приводится к числу.
   Результат NUMBER = арифметическому отрицанию результата приведения

См. также: <a href="#casting">casting</a>
</font>
</p><hr><p>
<font color="black"><a name="вызов метода"></a>
<a name="вызов функции"></a>
<a name="call_exp"></a>

<h3><font color="black">Вызов метода (функции)</font></h3>

Syntax: </font><font color="black"><b>explist = expr [ ',' explist ]</b></font><font color="black">
Syntax: </font><font color="black"><b>call_exp = ref '(' [explist] ')'</b></font><font color="black">

Вычисляет выражения перечисленные через запятую в скобках, формируя из их
значений список параметров. Если explist опущен - список пуст.
Вызывает метод/функцию, на которую ссылается ref, передавая в параметры
сформированый список (см. <a href="#ref">ref</a>, <a href="#expr">expr</a>).
</font>
</p><hr><p>
<font color="black"><a name="assign_exp"></a>
<a name="присваивание"></a>

<h3><font color="black">Присваивание</font></h3>

Syntax: </font><font color="black"><b>assign_exp = ref '=' expr</b></font><font color="black">

Вычисляет значение правой части и присваивает его ссылке, в левой части 
(см. <a href="#ref">ref</a>). Возвращаемое значение то же, что было получино в результате 
вычисления правой части. Присваивание - правоассоциативный оператор. 
Потому выражения можно выстраивать в цепочку. Так выражение 'a = b = c = 0'
выполняется следующим образом: '(a = (b = (c = (0))))'.
См. также <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="logical or"></a>
<a name="логическое или"></a>

<h3><font color="black">Логическое 'или'</font></h3>

Syntax: </font><font color="black"><b>expr '||' expr</b></font><font color="black">

Вычисляет левую часть. Результат вычисления приводится (см <a href="#casting">casting</a>) к 
логическому значению. Если это значение 'истина' - немедленно возвращает
результат 'истина.' Иначе - возвращает результат вычисления и приведения
к логическому значению правой части.

Т.е. результат вычисления выражения '1 || a()' всегда 'истина' и ф-я a() 
никогда не вычисляется.
См. также <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="logical and"></a>
<a name="логическое и"></a>

<h3><font color="black">Логическое 'и'</font></h3> 

Syntax: </font><font color="black"><b>expr '&&' expr</b></font><font color="black">

Вычисляет левую часть. Результат вычисления приводится (см <a href="#casting">casting</a>) к 
логическому значению. Если это значение 'ложь' - немедленно возвращает
результат 'ложь.' Иначе - возвращает результат вычисления и приведения
к логическому значению правой части.

Т.е. результат вычисления выражения '0 && a()' всегда 'ложь' и ф-я a() 
никогда не вычисляется.
См. также <a href="#expr">expr</a>
</font>
</p><hr><p>
<font color="black"><a name="приведение"></a>
<a name="casting"></a>

<h3><font color="black">Приведение типов</font></h3>

Феня не имеет специальных синтаксических структур для приведения одного 
типа к другому, однако, типы приводятся один к другому, в случае, если
это требуется контекстом. Неявное привевдение типов, например, необходимо для
бинарных операций, когда аргументы имеют различные типы, или как предикат в 
условных переходах. 

Действуют следующие правила приведения типов:
* число &lt;- NUMBER
* число &lt;- STRING - строка рассматривается как символьное представление 
                     числа 10ном коде
* строка &lt;- STRING
* строка &lt;- IDENTIFIER - строковое представление лексического идентификатора.
* строка &lt;- NUMBER - число записывается в виде строки в 10ном представлении.
* строка &lt;- FUNCTION - декомпиляция функции. преобразует пи-код 
                       скомпиллированной функции к строковому представлению
		       в соответствии с синтаксисом языка 
		       (обратное преобразование к parse).
* лог.выражение &lt;- NUMBER - false тогда и только тогда, когда число равно нулю.
* лог.выражение &lt;- STRING - false тогда и только тогда, когда строка пустая.
* идентификатор &lt;- INDENTIFIER
* сложный объект &lt;- OBJECT
* функция &lt;- FUNCTION
* все прочие попытки приведения вызывают InvalidCastException.
* транзитивные правила силы не имеют.
</font>
</p><hr></pre>Generated from areafile by feniatoxml.pl<br>Generated from xml by feniatohtml.xsl 