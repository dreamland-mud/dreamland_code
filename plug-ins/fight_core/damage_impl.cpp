/* $Id: damage_impl.cpp,v 1.1.2.6 2009/11/08 17:46:27 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "damage_impl.h"
#include "russianstring.h"

#include "skillreference.h"
#include "spell.h"
#include "skillgroup.h"
#include "character.h"
#include "merc.h"
#include "def.h"

GSN(resistance);
GSN(mental_knife);
GSN(dragons_breath);
GROUP(draconian);

/*-----------------------------------------------------------------------------
 * Self Damage
 *---------------------------------------------------------------------------*/
SelfDamage::SelfDamage( Character *ch, int dam_type, int dam ) : Damage( ch, ch, dam_type, dam )
{
}

void SelfDamage::calcDamage( )
{
    protectRazer( );
}
    
/*-----------------------------------------------------------------------------
 * Raw Damage
 *---------------------------------------------------------------------------*/
RawDamage::RawDamage( Character *ch, Character *victim, int dam_type, int dam )
    : Damage( ch, victim, dam_type, dam )
{
}

void RawDamage::message( )
{
    if( ch == victim ) {
	msgRoom( "%^C1\6себя", ch );
	msgChar( "Ты\5себя" );
	return;
    } 

    if ( dam == 0 ) {
       msgRoom( "%^C1\6%C2", ch, victim);
       msgChar( "Ты\5%C2", victim);
    }
    else {
       msgRoom( "%^C1\6%C4", ch, victim );
       msgChar( "Ты\5%C4", victim );
    }

    msgVict( "%^C1\6тебя", ch );
}

bool RawDamage::canDamage( )
{
    return true;
}

/*-----------------------------------------------------------------------------
 * SkillDamage 
 *----------------------------------------------------------------------------*/
SkillDamage::SkillDamage( Character *ch, Character *victim, 
		          int sn, int dam_type, int dam, bitstring_t dam_flag )
	    : Damage( ch, victim, dam_type, dam, dam_flag )
{
    this->sn = sn;
}

int SkillDamage::msgNoSpamBit( )
{
    return CONFIG_SKILLSPAM;
}

void SkillDamage::message( )
{
    const RussianString &attack = skillManager->find(sn)->getDammsg( );

    if (immune) {
	if (ch == victim) {
	    msgRoom("%1$^O1 %2$C2 бессил%1$Gьно|ен|ьна против %2$P4 сам%2$Gого|ого|ой|их", &attack, ch);
	    msgChar("Тебе повезло, у тебя иммунитет к этому");
	}
	else {
	    msgRoom("%1$^O1 %2$C2 бессил%1$Gьно|ен|ьна против %3$C2", &attack, ch, victim);
	    msgChar("%1$^T1 %1$O1 бессил%1$Gьно|ен|ьна против %2$C2", &attack, victim);
	    msgVict("Против тебя %2$O1 %1$C2 бессил%2$Gьно|ен|ьна", ch, &attack);
	}
    }
    else {
	if (ch == victim) {
	    msgRoom( "%1$^O1 %2$C2\6себя", &attack, ch );
	    msgChar( "%1$^T1 %1$O1\6тебя", &attack );
	}
	else {
	    if ( dam == 0 )
	    {
		msgRoom( "%1$^O1 %2$C2\6%3$C2", &attack, ch, victim );
		msgChar( "%1$^T1 %1$O1\6%2$C2", &attack, victim );
	    }
	    else {
		msgRoom( "%1$^O1 %2$C2\6%3$C4", &attack, ch, victim );
		msgChar( "%1$^T1 %1$O1\6%2$C4", &attack, victim );
	    }
	    msgVict( "%1$^O1 %2$C2\6тебя", &attack, ch );
	}
    }
}

/*
 * 'resistance' reduces 50% of non-magical damage,
 * with the historical exception for 'mental knife' spell
 */
void SkillDamage::protectResistance( )
{
    if (!victim->isAffected(gsn_resistance))
	return;

    if (sn == gsn_mental_knife) {
	dam -= victim->applyCurse( dam * 2 / 5 );
	return;
    }

    if (sn == gsn_dragons_breath) {
	return;
    }

    Skill *skill = skillManager->find( sn );
    Spell::Pointer spell = skill->getSpell( );

    if (!spell 
	    || !spell->isCasted( ) 
	    || spell->isPrayer( ch )
	    || skill->getGroup( ) == group_draconian)
    {
	dam -= victim->applyCurse( dam / 2 );
	return;
    }
}

