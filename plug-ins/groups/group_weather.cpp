
/* $Id: group_weather.cpp,v 1.1.2.11.6.8 2010-08-24 20:38:05 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "race.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "damage.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"


SPELL_DECL(CallLightning);
VOID_SPELL(CallLightning)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    int dam, vdam;

    if ( !RoomUtils::isOutside(ch) )
    {
        ch->pecho("Ты должен находиться вне помещения.");
        return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
        ch->pecho("Тебе нужна плохая погода.");
        return;
    }

    dam = dice(level, 9);

    ch->pecho("Божественная молния поражает твоих врагов!");
    act( "$c1 посылает молнию, которая повергает $s врагов!", ch, 0, 0, TO_ROOM );
    
    area_message( ch, "Молнии сверкают на небе.", false );

    for ( auto &vch : room->getPeople()) {

        if(!vch->isDead() && vch->in_room == room){

        if (vch->is_mirror() && number_percent() < 50) 
            continue;

        if (vch == ch)
            continue;
        
        if (is_safe_spell(ch, vch, true))
            continue;
        
        vdam = saves_spell( level, vch, DAM_LIGHTNING, ch, DAMF_PRAYER ) ? dam / 2 : dam;

        damage( ch, vch, vdam, sn, DAM_LIGHTNING, true, DAMF_PRAYER );

        }
    }
}

SPELL_DECL(ControlWeather);
VOID_SPELL(ControlWeather)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
        
    if (arg_oneof( target_name, "better", "лучше", "к лучшему" )) {
        weather_info.change += dice( level / 3, 4 );
        ch->pecho( "Прогноз погоды улучшается." );
    }
    else if (arg_oneof( target_name, "worse", "хуже", "к худшему" )) {
        weather_info.change -= dice( level / 3, 4 );
        ch->pecho( "Прогноз погоды ухудшается." );
    }
    else  {
        ch->pecho("Ты хочешь сделать погоду хуже или лучше?");
        return;
    }
}


SPELL_DECL(FaerieFire);
VOID_SPELL(FaerieFire)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) {
        if (victim == ch)
            act("{MРозовая аура{x уже окружает тебя.", ch, 0, 0, TO_CHAR);
        else
            act("$C1 уже окруже$Gно|н|на {Mрозовой аурой{x.", ch, 0, victim, TO_CHAR);
        return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level         = level;
    af.duration  = 10 + level / 5;
    af.location = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector.setValue(AFF_FAERIE_FIRE);
    affect_to_char( victim, &af );
    victim->pecho("Тебя окружает {MРозовая аура{x.");
    act_p( "$c4 окружает {MРозовая аура{x.",
            victim, 0, 0, TO_ROOM,POS_RESTING);
}


SPELL_DECL(FaerieFog);
VOID_SPELL(FaerieFog)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *ich;

    act_p("$c1 создает облако розового дыма.",
           ch, 0, 0, TO_ROOM,POS_RESTING);
    ch->pecho("Ты создаешь облако розового дыма.");

    for ( ich = room->people; ich != 0; ich = ich->next_in_room )
    {
        if (ich->invis_level > 0)
            continue;

        if ( ich == ch || saves_spell( level, ich,DAM_OTHER,ch, DAMF_SPELL|DAMF_WATER) )
            continue;

        affect_strip ( ich, gsn_invisibility                );
        affect_strip ( ich, gsn_mass_invis                );
        affect_strip ( ich, gsn_improved_invis                );
        strip_camouflage ( ich );
        REMOVE_BIT   ( ich->affected_by, AFF_HIDE        );
        REMOVE_BIT   ( ich->affected_by, AFF_FADE        );
        REMOVE_BIT   ( ich->affected_by, AFF_INVISIBLE        );
        REMOVE_BIT   ( ich->affected_by, AFF_IMP_INVIS        );        

        /* An elf sneaks eternally */
        if ( ich->is_npc() || !ich->getRace( )->getAff( ).isSet( AFF_SNEAK ))
          {
            affect_strip ( ich, gsn_sneak                       );
            REMOVE_BIT   ( ich->affected_by, AFF_SNEAK  );
          }

        act("$c1 обнаруже$gно|н|на!", ich, 0, 0, TO_ROOM);
        act("Тебя обнаружили!", ich, 0, 0, TO_CHAR);
    }
}


