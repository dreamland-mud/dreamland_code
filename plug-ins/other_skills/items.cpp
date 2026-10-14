#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "damage.h"
#include "loadsave.h"
#include "merc.h"

#include "act.h"
#include "def.h"
#include "l10n.h"

SPELL_DECL(GeneralPurpose);
VOID_SPELL(GeneralPurpose)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage_nocatch( ch, victim, dam, sn, DAM_PIERCE ,true, DAMF_WEAPON);
}


SPELL_DECL(HighExplosive);
VOID_SPELL(HighExplosive)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage_nocatch( ch, victim, dam, sn, DAM_PIERCE ,true, DAMF_WEAPON);
}


