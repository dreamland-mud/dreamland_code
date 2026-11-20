/* $Id$
 *
 * ruffina, 2004
 */
#include "ofcolguards.h"

#include "npcharacter.h"
#include "room.h"
#include "skillreference.h"

#include "act.h"
#include "fight.h"
#include "merc.h"

#include "loadsave.h"
#include "def.h"
#include "l10n.h"

GSN(garble);

bool OfcolMarshal::specFight( )
{
    Character *ach, *ach_next;
    
    BasicMobileDestiny::specFight();

    if (ch->in_room->areaIndex() != ch->zone) 
        return BasicMobileDestiny::specFight();

    if (number_percent() < 25) 
        return BasicMobileDestiny::specFight();
   
    yell_fmt( _("Охрана! На помощь!"), ch );

    if (ch->isAffected( gsn_garble ))
        return true;

    for( ach = char_list; ach != 0; ach = ach_next )
    {
        NPCharacter *mob;
        OfcolGuard::Pointer guard;

        if (!ch->fighting || ch->fighting->isDead())
            break;

        ach_next = ach->next;

        if (!ach->in_room
            || ach == ch
            || ach->in_room->area != ch->in_room->area
            || !ach->is_npc( ))
            continue;
        
        mob = ach->getNPC( );
        
        if (mob->fighting || mob->last_fought)
            continue;

        if (!mob->behavior)
            continue;

        guard = mob->behavior.getDynamicPointer<OfcolGuard>( );

        if (!guard)
            continue;

        if (mob->in_room == ch->in_room)
        {
            int i;

            oldact_p(_("$c1 призывает Богов на помощь."), ch,0,0,TO_ROOM,POS_SLEEPING);
            oldact_p(_("Боги призывают $c4 на помощь Диане."), mob,0,0,TO_ROOM,POS_SLEEPING);

            mob->max_hit = 6000;
            mob->hit = 6000;
            mob->setLevel( 60 );
            mob->damage[DICE_NUMBER] = number_range(3,5);
            mob->damage[DICE_TYPE] = number_range(12,22);
            mob->damage[DICE_BONUS] = number_range(6, 8);

            for(i=0;i<stat_table.size;i++)
                mob->perm_stat[i] = 23;

            say_fmt( _("Диана, я иду на помощь..."), mob );
            multi_hit( mob, ch->fighting , "murder" );
        }
        else {
            guard->pathToTarget( mob->in_room, ch->in_room, 2000 );

            if (!guard->path.empty( )) {
                if (number_percent() < 25)
                    yell_fmt( _("Держись, Диана! Я иду на помощь!"), mob );
                else
                    say_fmt( _("Диане необходима моя помощь."), mob );

                guard->makeOneStep( );
            }
        }
    }

    return true;
}

bool OfcolGuard::specFight()
{
    Character *ach, *ach_next;
    
    if (ch->in_room->areaIndex() != ch->zone) 
        return BasicMobileDestiny::specFight();

    if (number_percent( ) < 25) 
        return BasicMobileDestiny::specFight();

    if (!ch->fighting || ch->fighting->isDead())
        return BasicMobileDestiny::specFight();

    yell_fmt( _("Стража, на помощь! %2$^C1 убивает меня."), ch, ch->fighting );
   
    for (ach = char_list; ach != 0; ach = ach_next)
    {
        NPCharacter *mob;
        OfcolGuard::Pointer guard;

        if (!ch->fighting || ch->fighting->isDead())
            break;
            
        ach_next = ach->next;

        if (!ach->in_room
            || ach == ch
            || ach->in_room->area != ch->in_room->area
            || !ach->is_npc( ))
            continue;
        
        mob = ach->getNPC( );
        
        if (mob->fighting || mob->last_fought || mob->isDead( ))
            continue;

        if (!mob->behavior)
            continue;

        guard = mob->behavior.getDynamicPointer<OfcolGuard>( );

        if (!guard)
            continue;

        if (ch->in_room == mob->in_room) {
            say_fmt( _("Теперь, %2$C1, ты поплатишься за нападение на стражника."), mob, ch->fighting );
            multi_hit( mob, ch->fighting , "murder" );
        }
        else {
            guard->pathToTarget( mob->in_room, ch->in_room, 2000 );

            if (!guard->path.empty( )) {
                if (number_percent() < 25)
                    yell_fmt( _("Держись, стражник! Я иду на помощь."), mob );
                else
                    say_fmt( _("Страже необходима моя помощь."), mob );

                guard->makeOneStep( );
            }
        }
    }

    return true;
}


