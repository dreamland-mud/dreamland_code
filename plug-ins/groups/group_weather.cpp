
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
#include "race.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"


SPELL_DECL(CallLightning);
VOID_SPELL(CallLightning)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *vch;
    Character *vch_next;
    int dam, vdam;

    if ( !IS_OUTSIDE(ch) )
    {
        ch->send_to("Ты должен находиться вне помещения.\n\r");
        return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
        ch->send_to("Тебе нужна плохая погода.\n\r");
        return;
    }

    dam = dice(level, 9);

    ch->send_to("Божественная молния поражает твоих врагов!\n\r");
    act( "$c1 посылает молнию, которая повергает $s врагов!", ch, 0, 0, TO_ROOM );
    
    for (vch = room->people; vch; vch = vch_next) {
        vch_next = vch->next_in_room;

        if (vch->is_mirror() && number_percent() < 50) 
            continue;

        if (vch == ch)
            continue;
        
        if (is_safe_spell(ch, vch, true))
            continue;
        
        vdam = saves_spell( level, vch, DAM_LIGHTNING, ch, DAMF_SPELL ) ? dam / 2 : dam;
        damage( ch, vch, vdam, sn, DAM_LIGHTNING, true, DAMF_SPELL );
    }
    
    area_message( ch, "Молнии сверкают на небе.", false );
}

SPELL_DECL(ControlWeather);
VOID_SPELL(ControlWeather)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
        
    if (arg_oneof( target_name, "better", "лучше", "к лучшему" )) {
        weather_info.change += dice( level / 3, 4 );
        ch->println( "Прогноз погоды улучшается." );
    }
    else if (arg_oneof( target_name, "worse", "хуже", "к худшему" )) {
        weather_info.change -= dice( level / 3, 4 );
        ch->println( "Прогноз погоды ухудшается." );
    }
    else  {
        ch->send_to("Ты хочешь сделать погоду хуже или лучше?\n\r");
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

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level         = level;
    af.duration  = 10 + level / 5;
    af.location  = APPLY_AC;
    af.modifier  = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    victim->send_to("Тебя окружает {MРозовая аура{x.\n\r");
    act_p( "$c4 окружает {MРозовая аура{x.",
            victim, 0, 0, TO_ROOM,POS_RESTING);
}


SPELL_DECL(FaerieFog);
VOID_SPELL(FaerieFog)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *ich;

    act_p("$c1 создает облако розового дыма.",
           ch, 0, 0, TO_ROOM,POS_RESTING);
    ch->send_to("Ты создаешь облако розового дыма.\n\r");

    for ( ich = room->people; ich != 0; ich = ich->next_in_room )
    {
        if (ich->invis_level > 0)
            continue;

        if ( ich == ch || saves_spell( level, ich,DAM_OTHER,ch, DAMF_SPELL) )
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


