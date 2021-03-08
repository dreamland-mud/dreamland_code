#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "damage.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

SPELL_DECL(GeneralPurpose);
VOID_SPELL(GeneralPurpose)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage_nocatch( ch, victim, dam, sn, DAM_PIERCE ,true);
}


SPELL_DECL(HighExplosive);
VOID_SPELL(HighExplosive)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim, DAM_PIERCE) )
        dam /= 2;
    damage_nocatch( ch, victim, dam, sn, DAM_PIERCE ,true);
}

SPELL_DECL(Sebat);
VOID_SPELL(Sebat)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;

  if ( ch->isAffected(sn ) )
    {
      ch->send_to("Кассандра использовалась совсем недавно.\n\r" );
      return;
    }

  af.type      = sn;
  af.level     = level;
  af.duration  = level;
  af.location = APPLY_AC;
  af.modifier  = -30;
  affect_to_char( ch, &af );
  act( "Таинственный щит окружает $c4.",ch, 0,0,TO_ROOM);
  ch->send_to("Таинственный щит окружает тебя.\n\r");
  return;

}

SPELL_DECL(Matandra);
VOID_SPELL(Matandra)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  
  int dam;

  if ( ch->isAffected(sn ) )
    {
      ch->send_to("Кассандра использовалась для этой же цели совсем недавно.\n\r" );
      return;
    }

  postaffect_to_char( ch, sn, 5 );

  dam = dice(level, 7);
  damage_nocatch(ch,victim,dam,sn,DAM_HOLY, true);
}

SPELL_DECL(Kassandra);
VOID_SPELL(Kassandra)::run( Character *ch, Character *, int sn, int level ) 
{ 
    if ( ch->isAffected(sn ) )
      {
        ch->pecho("Ты совсем недавно пользовал%1$Gось|ся|ась этим заклинанием.", ch );
        return;
      }

    postaffect_to_char( ch, sn, 5 );

    ch->hit = min( ch->hit + 150, (int)ch->max_hit );
    update_pos( ch );

    ch->send_to("Волна тепла согревает твое тело.\n\r");
    act("$c1 выглядит лучше.", ch, 0, 0, TO_ROOM);
}


SPELL_DECL(DetectHidden);
VOID_SPELL(DetectHidden)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_HIDDEN) )
    {
        if (victim == ch)
          ch->send_to("Ты уже чувствуешь присутствие скрытых сил. \n\r");
        else
          act_p("$C1 уже чувствует присутствие скрытых сил.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }
    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (5 + level / 3);
    
    af.modifier  = 0;
    af.bitvector.setValue(DETECT_HIDDEN);
    affect_to_char( victim, &af );
    victim->send_to("Теперь ты чувствуешь присутствие скрытых сил.\n\r");
    if ( ch != victim )
        ch->send_to("Ok.\n\r");
    return;

}


