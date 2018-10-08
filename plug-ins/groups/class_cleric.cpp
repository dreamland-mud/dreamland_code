/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "magic.h"
#include "fight.h"
#include "handler.h"
#include "damage_impl.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"


SPELL_DECL(MindLight);
VOID_SPELL(MindLight)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af,af2;

    if ( room->isAffected( sn ))
    {
	ch->send_to("Эта комната уже полна энергетической силы.\n\r");
	return;
    }

    af.where     = TO_ROOM_CONST;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 30;
    af.location  = APPLY_ROOM_MANA;
    af.modifier  = level;
    af.bitvector = 0;
    room->affectTo( &af );

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level	 = level;
    af2.duration  = level / 10;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );
    ch->send_to("Ты наполняешь воздух энергетической силой, заставляя его мерцать.\n\r");
    act_p("$c1 наполняет воздух энергетической силой, заставляя его мерцать.",
           ch,0,0,TO_ROOM,POS_RESTING);
    return;

}

AFFECT_DECL(MindLight);
VOID_AFFECT(MindLight)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Воздух мерцает от избытка энергетической силы - "
                   "это на ближайшие {W%1$d{x ча%1$Iс|са|сов улучшит "
		   "восстановление маны на {W%2$d{x.",
		   paf->duration, paf->modifier )
	<< endl;
}

