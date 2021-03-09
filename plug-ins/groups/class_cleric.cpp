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
    Affect af;

    if ( room->isAffected( sn ))
    {
        ch->pecho("Эта комната уже полна энергетической силы.");
        return;
    }

    af.type      = sn;
    af.level     = level;
    af.duration  = level / 30;
    af.location.setTable(&apply_room_table);
    af.location = APPLY_ROOM_MANA;
    af.modifier  = level;
    room->affectTo( &af );

    postaffect_to_char(ch, sn, level/10);
    ch->pecho("Ты наполняешь воздух энергетической силой, заставляя его мерцать.");
    act("%^C1 наполняет воздух энергетической силой, заставляя его мерцать.",  ch, 0, 0,TO_ROOM);
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

