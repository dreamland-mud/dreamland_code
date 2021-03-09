/* $Id: scenarios.cpp,v 1.1.2.8.6.1 2007/09/29 19:34:06 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenarios.h"
#include "locatequest.h"
#include "questexceptions.h"

#include "selfrate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "interp.h"
#include "mercdb.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*-----------------------------------------------------------------------------
 * LocateScenario, default implementation
 *----------------------------------------------------------------------------*/
bool LocateScenario::applicable( PCharacter * ) const
{
    return true;
}

int LocateScenario::getCount( PCharacter *pch ) const
{
    if (rated_as_guru( pch ))
        return number_range( 6, 10 );
    else if (rated_as_newbie( pch ))
        return number_range( 2, 4 );
    else
        return number_range( 2, 10 );
}

void LocateScenario::actWrongItem( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest, Object *obj ) const
{
    oldact("$c1 произносит '{gСпасибо, конечно, но я не об этом проси$gло|л|ла тебя.'{x'", ch, 0, 0, TO_ROOM );
    act("%1$^C1 возвращает тебе %3$C4.", ch, hero, obj,TO_VICT);
    oldact("$c1 возвращает $C3 $o4.", ch, obj, hero, TO_NOTVICT );
}

void LocateScenario::actLastItem( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    act("%1$^C1 произносит '{gВот спасибо, %2$C1. Теперь все найдено и я могу спать спокойно.{x'",  ch, hero, 0,TO_ROOM);
    oldact("$c1 произносит '{gА вознаграждение я уже переда$gло|л|ла твоему квестору. Сходи и забери его.{x'", ch, 0, hero, TO_ROOM );
}

void LocateScenario::actAnotherItem( NPCharacter *ch, PCharacter *hero, LocateQuest::Pointer quest ) const
{
    if (chance(1) && quest->delivered == 1) {
        act("%^C1 произносит '{gДа-да, как говорится, еще 65535 ведер - и золотой ключик у нас в кармане.{x'", ch, hero, 0,TO_ROOM);
        interpret_raw( ch, "grin" );
        return;
    } 

    switch (number_range( 1, 3 )) {
    case 1:
        if (quest->delivered > 1) {
            oldact("$c1 произносит '{gО, ты наш$Gло|ел|ла еще $t!{x'", 
                    ch, russian_case( quest->itemName.getValue( ), '4' ).c_str( ), hero, TO_ROOM );
            break;
        }
        /* FALLTHROUGH */
    case 2:
        oldact("$c1 произносит '{gТеперь их уже $t, осталось совсем немного.{x'", 
                ch, DLString(quest->delivered).c_str( ), 0, TO_ROOM );
        break;
    case 3:
        interpret_fmt( ch, "nod %s", hero->getNameP( ) );
        break;
    }
}

