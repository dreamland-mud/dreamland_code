#include "character.h"
#include "room.h"
#include "commandtemplate.h"
#include "wrappertarget.h"
#include "wrapperbase.h"
#include "fight.h"
#include "arg_utils.h"
#include "descriptor.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

static void format_where( Character *ch, Character *victim )
{
    bool fPK, fAfk;
    
    fPK = (!victim->is_npc( ) 
            && victim->getModifyLevel( ) >= PK_MIN_LEVEL 
            && !is_safe_nomessage( ch, victim->getDoppel( ch ) ));
    fAfk = IS_SET(victim->comm, COMM_AFK);

    ch->pecho( "%-25C1 {x%s{x%s %-42s{x",
                victim,
                fPK  ? "({rPK{x)"  : "    ",
                fAfk ? "[{CAFK{x]" : "     ",
                victim->in_room->getName() );
}

static bool rprog_where( Character *ch, const char *arg )
{
    FENIA_CALL( ch->in_room, "Where", "Cs", ch, arg );
    return false;
}

CMDRUNP( where )
{
    Character *victim = 0;
    Descriptor *d;
    bool found;
    bool fPKonly = false;
    DLString arg( argument );

    ch->setWaitViolence( 1 );

    if (eyes_blinded( ch )) {
        ch->pecho( "Ты не можешь видеть вещи!" );
        return;
    }
    
    if (eyes_darkened( ch )) {
        ch->pecho( "Ты ничего не видишь! Слишком темно!" );
        return;
    }

    arg.stripWhiteSpace( );
    arg.toLower( );
    
    if (arg_is_pk( arg ))
        fPKonly = true;
    
    if (rprog_where( ch, arg.c_str( ) ))
        return;

    if (arg.empty( ) || fPKonly)
    {
        ch->pecho( "Ты находишься в местности {W{hh%s{x. Недалеко от тебя:",
                     ch->in_room->areaName().c_str() );
        found = false;

        for ( d = descriptor_list; d; d = d->next )
        {
            if (d->connected != CON_PLAYING)
                continue;
            if (( victim = d->character ) == 0)
                continue;
            if (victim->is_npc( ))
                continue;
            if (!victim->in_room || victim->in_room->area != ch->in_room->area)
                continue;
            if (IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
                continue;
            if (!ch->can_see( victim ))
                continue;
            if (fPKonly && is_safe_nomessage( ch, victim ))
                continue;
            
            found = true;
            format_where( ch, victim );
        }

        if (!found)
            ch->pecho("Никого.");
    }
    else
    {
        found = false;
        for ( victim = char_list; victim != 0; victim = victim->next )
        {
            if ( victim->in_room != 0
                    && victim->in_room->area == ch->in_room->area
                    && ( !victim->is_npc()
                    || ( victim->is_npc() && !IS_SET(victim->act, ACT_NOWHERE) ) )
                    && ch->can_see( victim )
                    && is_name( arg.c_str(), victim->getNameP( '7' ).c_str() )
                    && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
            {
                found = true;
                format_where( ch, victim );
            }
        }

        if (!found)
            oldact("Ты не находишь $T.", ch, 0, arg.c_str(), TO_CHAR);
    }
}


