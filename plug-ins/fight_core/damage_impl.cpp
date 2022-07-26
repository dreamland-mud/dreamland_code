/* $Id: damage_impl.cpp,v 1.1.2.6 2009/11/08 17:46:27 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "damage_impl.h"
#include "damageflags.h"

#include "logstream.h"
#include "russianstring.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "affect.h"
#include "affecthandler.h"
#include "skillreference.h"
#include "spell.h"
#include "skillgroup.h"
#include "npcharacter.h"
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
RawDamage::RawDamage( Character *ch, Character *victim, int dam_type, int dam, const DLString &deathReason )
    : Damage( ch, victim, dam_type, dam )
{
    this->deathReason = deathReason;
}

void RawDamage::message( )
{
    if( ch == victim ) {
        msgRoom( "%2$^C1\6себя", dam, ch );
        msgChar( "Ты\5себя", dam );
        return;
    } 

    if ( dam == 0 ) {
       msgRoom( "%2$^C1\6%3$C2", dam, ch, victim); 
       msgChar( "Ты\5%3$C2", dam, ch, victim);
    }
    else {
        msgRoom( "%2$^C1\6%3$C4", dam, ch, victim );
        msgChar( "Ты\5%3$C4", dam, ch, victim );
    }

    msgVict( "%2$^C1\6тебя", dam, ch );
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
    this->deathReason = skillManager->find(sn)->getName();
}

SkillDamage::SkillDamage( Affect *paf, Character *victim, int sn, int dam_type, int dam, bitstring_t dam_flag )
            : Damage( paf, victim, dam_type, dam, dam_flag )
{
    this->sn = sn;
    this->deathReason = skillManager->find(sn)->getName();
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
            msgRoom("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %3$P4 сам%3$Gого|ого|ой|их", dam, &attack, ch);
            msgChar("Тебе повезло, у тебя иммунитет к этому", dam);
        }
        else {
            msgRoom("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %4$C2", dam, &attack, ch, victim);
            msgChar("%2$^T1 %2$O1 бессил%2$Gьно|ен|ьна|ьны против %3$C2", dam, &attack, victim);
            msgVict("Против тебя %3$O1 %2$C2 бессил%3$Gьно|ен|ьна|ьны", dam, ch, &attack);
        }
    }
    else {
        if (ch == victim) {
            msgRoom( "%2$^O1 %3$C2\6%3$P2", dam, &attack, ch );
            msgChar( "%2$^T1 %2$O1\6тебя", dam, &attack );
        }
        else {
            if ( dam == 0 )
            {
                msgRoom( "%2$^O1 %3$C2\6%4$C2", dam, &attack, ch, victim );
                msgChar( "%2$^T1 %2$O1\6%3$C2", dam, &attack, victim );
            }
            else {
                msgRoom( "%2$^O1 %3$C2\6%4$C4", dam, &attack, ch, victim );
                msgChar( "%2$^T1 %2$O1\6%3$C4", dam, &attack, victim );
            }

            msgVict( "%2$^O1 %3$C2\6тебя", dam, &attack, ch );
        }
    }
}


bool SkillDamage::mprog_immune()
{
    DLString damType = damage_table.name(dam_type);
    DLString sname = skillManager->find(sn)->getName();
    FENIA_NUM_CALL(victim, "Immune", dam, "CisOis", ch, dam, damType.c_str(), NULL, dam_flag, sname.c_str());
    FENIA_NDX_NUM_CALL(victim->getNPC(), "Immune", dam, "CCisOis", victim, ch, dam, damType.c_str(), NULL, dam_flag, sname.c_str());

    for (auto &paf: victim->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect()->onImmune(SpellTarget::Pointer(NEW, victim), paf, ch, dam, damType.c_str(), NULL, dam_flag, sname.c_str()))
            return true;

    return false; 
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
        dam -= ( dam * 2 / 5 );
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
            || skill->hasGroup(group_draconian))
    {
        dam -= ( dam / 2 );
        return;
    }
}

