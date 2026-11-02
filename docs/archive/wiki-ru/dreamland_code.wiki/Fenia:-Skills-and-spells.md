# Заклинания

Для всех заклинаний можно перекрыть их логику из фени, объявив один из методов: `runVict`, `runChar`, `runArg` или `runRoom`. Выбор метода происходит в коде команды `cast` и зависит от настроек заклинания (его поля `target`) и того, с каким параметром персонаж применил данное заклинание.

Например, для заклинания `bless` в его профайле указано:
```xml
    <target>char_room obj_inv obj_room</target>
```
Это означает, что благословить можно как персонажа, так и предмет в своем инвентаре или рядом в комнате. Если выполнена команда `c bless barrel`, вызовется метод `runObj`, а если выполнить `c bless` или `c bless grigoriy`, вызовется метод `runVict`.

Для заклинаний, у которых есть несколько таких методов run, можно перекрыть как все, так и только некоторые -- в последнем случае будет просто вызван исходный метод из С++.

Для перекрытия логики заклинания достаточно в сценарии выполнить такое присваивание:
```javascript
.Spell("bless").runObj = function() { 
    ch.act("Заклинание {g%s{x вызвано для {W%O2{x на уровне {C%d{x.", spell.rname, obj, level); 
}
```
После этого исходный метод из С++ вызываться не будет, даже если внутри `runObj` пустота, будьте осторожны.

Самый простой способ перекрыть заклинание из фени -- воспользоваться редактором умений `skedit` и его командой `fenia runObj`, `fenia runVict`. При этом тема сценария будет автоматически установлена в `spell/bless/runObj`, что позволит легко искать все такие методы командой `cs list <фильтр>`. Сценарии заклинаний автоматически сохраняются на диск в каталоге `fenia/spell/...`.

## Поля и методы, доступные изнутри методов run

Для краткости записи в методы run* ничего не передается в параметры, вместо этого все необходимые поля и методы доступны изнутри run* (так называемый контекст вызова заклинания, который становится полем `this`).
Полный список доступных полей и методов можно найти [на сайте во вкладке Умения](https://dreamland.rocks/feniaapi.html#skillspell).
Вот несколько примеров:

* `vict` - цель заклинания для метода `runVict`, в других методах равна `null`. Это поле можно переприсвоить, тогда поменяется и цель заклинания. Вспомогательные методы наподобие `damage()`, `savesSpell` всегда работают с текущим `vict`.
* `obj` - цель заклинания для метода `runObj`, в других методах равна `null`.
* `arg` - цель заклинания для метода `runArg`, в других методах равна `null`.
* `room` - цель заклинания для метода `runRoom`, в других методах равна `null`.
* `dam` - текущие повреждения. Устанавливаются как напрямую: `dam = dam + 30;`  так и через вызов методов `calcDamage()`. Метод `calcDamage` автоматически вызовется для каждого витка цикла у функции `damageRoom` или в самом начале метода `runVict`.


## Примеры заклинаний

### Метод runVict с побочными эффектами и нестандартной логикой

```javascript
.Spell("bluefire").runVict = function() {
    if (!ch.is_npc() && !ch.neutral) {
        vict = ch;
        msgChar("Твой {CГолубой огонь{x оборачивается против тебя!");
    }

    if (.chance(50)) {
        savesSpell(.tables.damage_table.fire);

        if (vict != ch) {
            msgNotVict("Голубой огонь %1$C2 {Rобжигает{x %2$C4!");
            msgVict("Голубой огонь %1$C2 {Rобжигает{x тебя!");
            msgChar("Твой Голубой огонь {Rобжигает{x %2$C4!");
        } else {
            msgChar("Твой Голубой огонь {Rобжигает{x тебя!");
            msgRoom("%1$^C1 {Rобжигается{x своим голубым огнем.");
        }

        damage(.tables.damage_table.fire);
        effectFire();

    } else {
        savesSpell(.tables.damage_table.cold);

        if (vict != ch) {
            msgNotVict("Голубой огонь %1$C2 {Cобмораживает{x %2$C4!");
            msgVict("Голубой огонь %1$C2 {CCобмораживает{x тебя!");
            msgChar("Твой Голубой огонь {Cобмораживает{x %2$C4!");
        } else {
            msgChar("Твой Голубой огонь {Cобмораживает{x тебя!");
            msgRoom("%1$^C1 {Cобмораживается{x своим голубым огнем.");
        }

        damage(.tables.damage_table.cold);
        effectCold();
    }
}

```

### Метод runRoom c нестандартным вычислением повреждений

```javascript
.Spell("earthquake").runRoom = function() {
    msgChar("Земля дрожит под твоими ногами!");
    msgRoom("%1$^C1 вызывает ураган и землетрясение.");
    msgArea("Земля слегка дрожит под твоими ногами.");

    damageRoom(function() { // not ch, not safe, 50% chance for mirrors
        if (vict.flying)
            return;

        if (vict.in_room.sector_type == .tables.sector_table.mountain)
            dam = dam * 4;
        else if (vict.in_room.sector_type == .tables.sector_table.city)
            dam = dam * 3;
        else if (vict.in_room.sector_type == .tables.sector_table.inside)
            dam = dam * 2;

        damage();

        if (.chance(skill.effective(ch) / 2)) {
            vict.wait = 2 * 12;
            vict.position = .tables.position_table.rest;
        }
    });

    // Destroy grave belonging to a vampire in owr PK range.
    if (graveFind() != null && .chance(skill.effective(ch) / 2)) {
	graveDestroy();
    }
}
```

### Метод runRoom с применением эффектов на комнату и на окружающих

```javascript
.Spell("scream").runRoom = function() {

	if (ch.isAffected("scream")) {
		ch.act("Ты сдавленно хрипишь.");
		return;
	}
	
	msgChar("Ты пронзительно кричишь, сотрясая все вокруг!");
	msgRoom("%1$^C1 пронзительно кричит, сотрясая все вокруг!");  
	msgArea("До тебя доносятся пронзительные вопли.");
	
	dam = .dice(level, 20); // TODO calc damage from caster's hp
	effectScream(); // called for the whole room
	
	// Remember original level and damage.
	state.level = level;
	state.dam = dam;
	
	damageRoom(function() { // called for all unsafe victims in the room
	
		// Reset level and damage back to original for every victim.
		level = state.level; 
		dam = state.dam;
		
		if (!savesSpell())
			vict.wait = 2 * 12; // 2 battle ticks
		else {
			dam = dam / 2; // divide by 2 again to get dam/4.
			level = level / 2;
		}
		
		effectScream(); // called for the current victim
	});  
}
```

### Метод runObj с наложением аффекта на предмет

```javascript
.Spell("hunger weapon").runObj = function() {
    var chance;

    if (obj.item_type != .tables.item_table.weapon) {
        msgChar("%2$^O1 не оружие.");
        return;
    }

    if (obj.value4 & .tables.weapon_type2.vampiric) {
        msgChar("%2$^O1 уже жажд%2$nет|ят чужой крови.");
        return;
    }

    if (obj.value4 & .tables.weapon_type2.holy
        || obj.extra_flags & .tables.extra_flags.anti_evil
        || obj.extra_flags & .tables.extra_flags.bless)
    {
        msgChar("Боги Света наказывают тебя за попытку осквернения священного оружия!");
        msgRoom("Боги Света наказывают %1$C4 за попытку осквернения священного оружия!");
        dam = .min(ch.hit - 1, 1000);
        ch.rawdamage(ch, dam, "holy");
        return;
    }
        
    chance = skill.effective(ch);
    if (obj.value4 & .tables.weapon_type2.flaming)
        chance = chance / 2;
    if (obj.value4 & .tables.weapon_type2.frost)
        chance = chance / 2; 
    if (obj.value4 & .tables.weapon_type2.sharp)
        chance = chance / 2;
    if (obj.value4 & .tables.weapon_type2.vorpal)
        chance = chance / 2;
    if (obj.value4 & .tables.weapon_type2.shocking)
        chance = chance / 2;
    if (obj.value4 & .tables.weapon_type2.fading)
        chance = chance / 2;

    if (.number_percent() >= chance) {
        msgAll("%2$^O1 на миг ярко вспыхива%2$nет|ют багровым... но затем угаса%2$nет|ют.");
        return;
    }
       
    var af;
    af = .Affect();
    af.type = skill.name;
    af.level = level / 2;
    af.duration = level / 4;
    af.bitvector("weapon_type2", "vampiric");
    obj.affectAdd(af);

    af.bitvector("extra_flags", "anti_good anti_neutral");
    obj.affectAdd(af);

    msgChar("Ты передаешь %2$O3 свою жажду крови.");
    msgRoom("%1$^C1 внимательно смотрит на %2$O4, %1$P2 глаза вспыхивают {rкрасным{x.");
}
```

### Метод runObj, заставляет вампира выбраться из могилы

```javascript
.Spell("entangle").runObj = function() {
	if (obj != graveFind()) {
		// Either obj is not a grave at all, or vampire is safe from ch.
		msgChar("%2$^O1 неподходящая цель.");
		return;
	}
	
	// TODO: saves check or chance check
	
	// Success:
	vict = graveOwner(); // assign vict field so that damage() and msgVict() work
	msgVict("Корни терновника проникают в могилу, тревожа твой покой.");
	msgAll("Колючий терновник опутывает могилу, проникая корнями глубоко под землю!");
	graveDestroy();
	
	// Vampire is now here, do some damage.
	dam = .number_range(level, level * 4);
	damage("pierce");
}

```

### Метод runVict, наносящий повреждение всем предметам жертвы 
```javascript
.Spell("heat metal").runVict = function() {

    dam = 0;
    state.success = false; // Track successful heat attempts, false by default.

    if (!savesSpell())
        damageItems(function () {
            if (.number_range(1, 2 * level) <= obj.level)
                return;

            if (!obj.madeOfMetal())
                return;

            if (obj.extra_flags & .tables.extra_flags.burn_proof)
                return;

            if (obj.item_type == .tables.item_table.weapon
                && obj.value4 & .tables.weapon_type2.flaming)
                return;

            if (savesSpell())
                return;

            state.success = true;

            // Drop from inventory or burn.
            if (obj.wear_loc == "none") {
                dam = dam + .number_range(1, obj.level) / 2;

                if (vict.can_drop_obj(obj)) {
                    // TODO: do we really need different msgs for weapon and non-weapon
                    if (obj.item_type == .tables.item_table.weapon) {
                        vict.act("%1$^O1 раскаля%1$nется|ются, и ты бросаешь %1$P2 на землю.", obj);
                        vict.recho("%1$^O1 раскаля%1$nется|ются, и %2$C1 бросает %1$P2 на землю.", obj, vict);
                    } else {
                        vict.recho("%1$^C1 кричит от боли и бросает %2$O4 на землю!", vict, obj);
                        vict.act("Ты кричишь от боли и бросаешь %O4 на землю!", obj);
                    }

                    obj.obj_to_room(vict.in_room);
                    return;
                }

                // Stuck in inventory.
                if (obj.item_type == .tables.item_table.weapon)
                    vict.act("%1$^O1 обжига%1$nет|ют тебя!", obj);
                else
                    vict.act("%1$^O1 обжига%1$nет|ют твою кожу!", obj);

                return;
            }

            // TODO: calculate chance to drop an equipped item.
            // if ((obj.weight / 10) > number_range(1, 2 * vict.cur_dex))
            // TODO: item weight should affect dam.
            // TODO: first should try to remove() and put in inventory, then try to drop.

            // Worn or wielded item: remove & drop or burn.
            if (vict.can_drop_obj(obj) && obj.remove()) {
                dam = dam + .number_range(1, obj.level) / 2;

                if (obj.item_type == .tables.item_table.weapon) {
                    vict.recho("%1$^O1 выпада%1$nет|ют из обожженных рук %2$C2.", obj, vict);
                    vict.act("Оружие выпадает из твоих обожженных рук!");
                } else {
                    vict.recho("%1$^C1 кричит от боли и бросает %2$O4 на землю!", vict, obj);
                    vict.act("Ты кричишь от боли и бросаешь %O4 на землю!", obj);
                }

                obj.obj_to_room(vict.in_room);
                return;
            }

            // Stuck on the body!
            dam = dam + .number_range(1, obj.level);

            if (obj.item_type == .tables.item_table.weapon)
                vict.act("Раскаленное оружие обжигает твои руки!");
            else
                vict.act("%1$^O1 обжига%1$nет|ют твою кожу!", obj);
        });

    if (state.success) {
        // Inflict accumulated damage. Set 'damtype fire' in skedit.
        dam = dam * 2 / 3;
        damage();
    } else {
        // Oops nothing happened.
        msgChar("Твоя попытка нагревания закончилась неудачей.");
        msgVict("Ты чувствуешь легкое прикосновение тепла.");
    }
}


```
### Метод runRoom, меняющий тип местности в комнате
```javascript
.Spell("deforestation").runRoom = function() {
	var af;
	
	if (sect != "forest") {
		ch.act("Вокруг тебя нет леса, который можно уничтожить.");
		return;
	}

	af = .Affect();
	af.type      = skill.name;
	af.level     = level;
	af.duration  = level/25 + 1;
	ch.affectAdd(af); 	
	af.location  = .tables.apply_flags.sector_type;
	af.modifier  = .tables.sector_table.field;
	room.affectJoin(af);
	
	ch.in_room.echo("Деревья вокруг тебя исчезают, и местность превращается в поле.");
}
```

***

# Работа с командами умений из Фени

Многим умениям, таким как 'подножка', 'бросок грязью', 'воровство', соответствуют команды (с таким же или чуть измененным названием, например, 'грязь', 'украсть'). Конфигурация таких команд задается внутри профайлов их умений, в секции `<command>`, например:

```xml
  <command type="SKILL(dirt)"> 
    <cat>class</cat> 
    <hint>(воинские профессии) ослепить врага грязью</hint> 
    <name>dirt</name> 
    <order>fight_only</order> 
    <position>fight</position> 
    <russian> 
      <node>грязь</node> 
    </russian> 
  </command> 
```

Команды умений обладают почти теми же полями и свойствами, что и обычные команды. Большинство логики для них на данный момент задано в коде, однако уже есть возможность переопределить их метод `run` из фени, и тогда будет вызван именно он. Это удобно делать изнутри редактора `skedit`, пользуясь подкомандой `fenia run`.

Важным полем для команды умения является `argtype`, равное одному из значений таблицы `argtype_table`. Установив это поле в `obj_here` или `char_room`, можно добиться того, что внутрь метода `run` попадет уже заранее найденный предмет `obj` или персонаж `vict`, а если они не найдутся -- игроку выдастся сообщение об ошибке. Если `argtype` не задан, аргументы придется анализировать самостоятельно, используя поля `argAll`, `argOne` и `argTwo`.

## Поля и методы, доступные изнутри метода run

Для краткости записи в метод `run` ничего не передается в параметры, вместо этого все необходимые поля и методы сразу доступны изнутри `run`. Полный список доступных полей и методов можно найти [на сайте во вкладке Умения](https://dreamland.rocks/feniaapi.html#skillspell). Вот несколько примеров:

* `argAll` - вся строка аргументов, с которыми была вызвана эта команда
* `argOne` - первый аргумент, с которым была вызвана команда
* `argTwo` - второй аргумент, с которым была вызвана команда
* `vict` - персонаж, цель данной команды. Будет определен для команд, у которых `argtype` равен `char_fight`, `char_room` и так далее
* `obj` - предмет, цель данной команды. Будет определен для команд, у которых `argtype` равен `obj_here`, `obj_carry` и так далее

## Примеры команд

Команда без аргументов, вешает аффект на комнату.

```javascript
.SkillCommand("settraps").run = function() {

	if (!skill.usable(ch)) {
		ch.act("Ты абсолютно не в курсе, как это делается.");
		return;
	}
	
	if (ch.in_room.room_flags & .tables.room_flags.law) {
		ch.act("Мистическая сила защищает комнату.");
		return;
	}

	if (ch.in_room.isAffected(skill.name)) {
		ch.act("В этой комнате уже есть ловушка.");
		return;
	}
	
	if (ch.isAffected(skill.name)) {
		ch.act("Ты не сможешь уследить за двумя ловушками.");
		return;
	}

	ch.wait = skill.beats(ch);

	if (.chance(7 * skill.effective(ch) / 10)) {
		var af;
		
		af = .Affect();
		af.type = skill.name;
		af.level = skill.level(ch);
		af.duration = af.level / 40;
		af.where = .tables.affwhere_flags.room_affects;
		af.bitvector = .tables.raffect_flags.thief_trap;
		ch.in_room.affectJoin(af);
		
		cooldown(af.duration);

		ch.act("Ты устраиваешь ловушку в комнате.");
		ch.recho("%^C1 устраивает ловушку в комнате.", ch);
				
		skill.improve(ch, true);
		
	} else {
		ch.act("Твоя попытка устроить ловушку провалилась.");
		skill.improve(ch, false);
	}
}
```

***

# Работа с временными умениями 

С помощью Фени можно дать персонажу какое-то временное умение, ранее не доступное для его профессии, как будто оно ему приснилось.

Если это умение уже было доступно (как профессиональное или уже приснилось), метод ничего не сделает и вернет false.

Пример присваивания временного умения в триггере `onEquip`:
```javascript
if (.Skill("dodge").giveTemporary(ch, 75)) {
    // Успешно повесилось разученным на 75, пока не снимет вещь.
    ch.act("Ты чувствуешь себя ловким и проворным, как лиса.");
}
```

Или так:
```javascript
.Skill("dodge").giveTemporary(ch, 100, 3); // повесить разученным на 100, на 3 мировых дня.
```

Снять такое умение можно в триггере `onRemove`. Никаких проверок не нужно, лишнего не снимет.

```javascript
.Skill("dodge").removeTemporary(ch);
```

Cправка по всем методам скилла можно вывести так:
```javascript
eval ptc(.Skill("none").api())
```

## Создание новых умений из фени (устарело, используй skedit)

Для этого служит конструктор `.FeniaSkill("название")`. Созданное умение существует, пока на него кто-то ссылается (т.е. если просто внутри функции объявить его во временной переменной, то умение исчезнет как только закончится выполняться функция). Поэтому удобно присвоить новое умение куда-то в .tmp, где его никто не разрушит.

Для удобства введем `.tmp.skills` как хранилище всех феневых умений, но это не обязательно, это может быть любое поле.

Зарегистрировать новое умение и прописать ему имена:
```javascript
if (.tmp.skills == null)
    .tmp.skills = .Map();

.tmp.skills.cowardice = .FeniaSkill("cowardice");
.tmp.skills.cowardice.nameRus = "трусость";
.tmp.skills.cowardice.dammsg = "трусость";
.tmp.skills.cowardice.dammsg_gender = "f";
```

Повесить на себя вечный аффект трусости с правильным русским именем:
```javascript
eval affectAdd(.Affect("cowardice", 100, -1))
```

Нанести себе 10 очков повреждений, 'Твоя трусость царапает тебя.':
```javascript
eval damage(this, 10, "cowardice", "none")
```

Посмотреть все поля:
```javascript
eval ptc(.tmp.skills.cowardice.api())
```

Пока это все скилы-пустышки, которые годятся для повесить аффект или нанести повреждения. По мере надобности добавим возможность cоздавать заклинания или боевые умения-команды.


## Аффекты, влияющие на знания

На персонажа можно повесить аффект, улучшающий/ухудшающий знание умения или целой группы умений.

Пример аффекта, влияющего на знания нескольких умений:

```javascript
var af;

af = .Affect();
af.type = "clumsiness";                       // (создать новое умение как описано выше)
af.location = .tables.apply_flags.learned;    // на что оказываем влияние (разученность)
af.modifier = -50;                            // нету у тебя больше парирования
af.where = .tables.affwhere_flags.skills;     // что именно меняем - умения или группу умений
af.global = "dodge parry 'enhanced damage'";  // действует на несколько умений сразу
af.duration = 10;                             // на 10 минут (тиков)
af.level = ch.level;
ch.affectAdd(af);
```

Пример аффекта, влияющего на знание группы умений:

```javascript
af.where = .tables.affwhere_flags.skill_groups;
af.modifier = 100;
af.global = "benedictions"; // благословляй не хочу
```

## Аффекты, влияющие на уровень заклинаний

На персонажа можно повесить аффект, влияющий на уровень произносимых заклинаний. Влиять можно на все заклинания, заклинания одной группы или индивидуальные.

Пример аффекта, влияющего на уровень всех заклинаний:
```javascript
var af;

af = .Affect();
af.type = "abracadabra"; 
af.location = .tables.apply_flags.level       // на что оказываем влияние (уровень)
af.modifier = 3;                              // увеличивать уровень на 3
af.duration = 10;                             // на 10 минут (тиков)
af.level = ch.level;
ch.addAffect(af);
```

Пример аффекта, влияющего на уровень лечебных заклинаний:
```javascript
var af;

af = .Affect();
af.type = "abracadabra"; 
af.location = .tables.apply_flags.level       // на что оказываем влияние (уровень)
af.modifier = 1;                              // увеличивать уровень на 1
af.where = .tables.affwhere_flags.skill_groups;
af.global = "healing";
af.duration = 10;     
af.level = ch.level;
ch.addAffect(af);
```

## См. также

* [Работа с аффектами умений в OLC](https://github.com/dreamland-mud/dreamland_code/wiki/OLC%3A-oedit-addaffects-for-skills)