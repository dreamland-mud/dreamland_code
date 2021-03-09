/* $Id: mlove.cpp,v 1.1.2.14.6.5 2009/11/04 03:24:33 rufina Exp $
 * ruffina, 2003
 * ideas and beta-testing with Filths >8) 
 */

#include "logstream.h"
#include "commandtemplate.h"

#include "xmllovers.h"
#include "lover.h"
#include "xmlattributelovers.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

#define MLOVE_DAZE(ch)        (ch)->wait = std::max((ch)->wait, 25);

static bool mprog_makelove( Character *ch, Character *victim )
{
    FENIA_CALL( ch, "MakeLove", "C", victim );
    FENIA_NDX_CALL( ch->getNPC( ), "MakeLove", "CC", ch, victim );
    return false;
}

CMDRUN( mlove )
{
        DLString arguments = constArguments;
        DLString arg;
        std::basic_ostringstream<char> str;
        Character *victim;

        if (IS_CHARMED(ch)) {
            oldact("... но сердцу не прикажешь.", ch, 0, 0, TO_CHAR);  
            oldact("$c1 ухмыляется - сердцу не прикажешь.", ch, 0, ch->master, TO_VICT);
            return;
        }

        if (arguments.empty( )) {
            if (ch->getSex( ) == SEX_MALE)
                oldact("Ты никак не можешь определиться: куда совать жетон?", ch, 0, 0, TO_CHAR);
            else 
                oldact("Куда пойти, куда податься.. кого найти, кому отдаться?", ch, 0, 0, TO_CHAR);

            oldact("$c1 гоняется с похотливым видом за всеми в комнате..берегись!", ch, 0, 0, TO_ROOM);
            return;
        }
        
        arg = arguments.getOneArgument( );

        if ( (victim = get_char_room(ch, arg.c_str())) == 0 ) {
            ch->pecho("Объект твоей страсти куда-то подевался.");
            return;
        }

        if (ch == victim) {
            ch->move -= ch->move / 4;
            ch->mana -= ch->mana / 4;

            ch->pecho("Да! Ты любишь себя! Еще, еще..!");
            oldact("Страсть $c1 к само$gму|му|й себе пользуется полной взаимностью.", ch, 0, 0, TO_ROOM);
            MLOVE_DAZE(ch);
            return;
        }

        if (ch->position == POS_FIGHTING) {
            if (ch->getSex( ) == SEX_MALE)
                oldact("Быстро спрячь, пока не отрезали!", ch, 0, 0, TO_CHAR);
            else 
                oldact("Эй, не отвлекайся!", ch, 0, 0, TO_CHAR);
            
            oldact("$c1 торжественно произносит: '{gMake love, not war!{x'", ch, 0, 0, TO_ROOM);
            return;
        }

        if (victim->position <= POS_STUNNED) {
            oldact("$M сейчас 'как-то так'.. извини.", ch, 0, victim, TO_CHAR);
            return;
        }
        else if (victim->position == POS_SLEEPING) {
            oldact("Может, стоит $S для начала разбудить?", ch, 0, victim, TO_CHAR);
            oldact("$c1 вертится вокруг $C2 и так, и эдак, но что-то $s смущает. Наверное, $S храп?", ch, 0, victim, TO_NOTVICT);
            return;
        }
        else if (victim->position == POS_FIGHTING) {
            oldact("$M сейчас совсем не до тебя.", ch, 0, victim, TO_CHAR);
            return;
        }
        
        if (mprog_makelove( ch, victim ))
            return;

        if (ch->is_npc()) {
            ch->pecho("Тебе нельзя.");
            return;
        }

        if (mlove_accepts(ch, victim)) {        
            ch->mana -= ch->mana / 4;
            victim->mana -= victim->mana / 4;
            ch->move -= ch->move / 4;
            victim->move -= victim->move / 4;

            oldact("Ты срываешь с $C2 одежду и страстно занимаешься с $Y любовью.", ch, 0, victim, TO_CHAR);
            oldact("$c1 срывает с тебя одежду и страстно занимается с тобой любовью. Ах, да! Еще, еще!", ch, 0, victim, TO_VICT);
            oldact("$c1 срывает с $C2 одежду и страстно занимается с $Y любовью.", ch, 0, victim, TO_NOTVICT);
            
            MLOVE_DAZE(victim);
            MLOVE_DAZE(ch);
            
            return;
        }
        
        oldact("О$Gно|н|на тебя не хочет.", ch, 0, victim, TO_CHAR);
        oldact("$c1 пытается добиться от тебя взаимности, но ты отвергаешь $s.", ch, 0, victim, TO_VICT);
        oldact("$c1 пытается добиться от $C2 взаимности, но $C1 отвергает $s.", ch, 0, victim, TO_NOTVICT);
}


