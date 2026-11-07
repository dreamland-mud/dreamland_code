/* $Id$
 *
 * ruffina, 2004
 */
#include "immunity.h"
#include "damageflags.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

void immune_from_flags( Character *ch, bitstring_t bit, int &res )
{
    if (IS_SET(ch->imm_flags, bit))
        SET_BIT(res, RESIST_IMMUNE);

    if (IS_SET(ch->res_flags, bit))
        SET_BIT(res, RESIST_RESISTANT);

    if (IS_SET(ch->vuln_flags, bit))
        SET_BIT(res, RESIST_VULNERABLE);
}

int immune_resolve( int res )
{
    if (IS_SET(res, RESIST_IMMUNE))
        return RESIST_IMMUNE;

    if (IS_SET(res, RESIST_VULNERABLE) && IS_SET(res, RESIST_RESISTANT))
        return RESIST_NORMAL;
    
    return res;
}

bitstring_t immune_bit( int dam_type )
{
    /* VULN etc. bits must ALL be the same for this to work */
    switch (dam_type)
    {
        case(DAM_BASH):                return IMM_BASH;
        case(DAM_PIERCE):        return IMM_PIERCE;
        case(DAM_SLASH):        return IMM_SLASH;
        case(DAM_FIRE):                return IMM_FIRE;
        case(DAM_COLD):                return IMM_COLD;
        case(DAM_LIGHTNING):        return IMM_LIGHTNING;
        case(DAM_ACID):                return IMM_ACID;
        case(DAM_POISON):        return IMM_POISON;
        case(DAM_NEGATIVE):        return IMM_NEGATIVE;
        case(DAM_HOLY):                return IMM_HOLY;
        case(DAM_ENERGY):        return IMM_ENERGY;
        case(DAM_MENTAL):        return IMM_MENTAL;
        case(DAM_DISEASE):        return IMM_DISEASE;
        case(DAM_DROWNING):        return IMM_DROWNING;
        case(DAM_LIGHT):        return IMM_LIGHT;
        case(DAM_CHARM):        return IMM_CHARM;
        case(DAM_SOUND):        return IMM_SOUND;
        default:                return 0;
    }
}

int immune_check(Character *ch, int dam_type, bitstring_t dam_flag)
{
    int res;

    res = RESIST_NORMAL;

    if (dam_type == DAM_NONE)
        return res;
    
    if (IS_SET(dam_flag, DAMF_WEAPON)) 
        immune_from_flags( ch, IMM_WEAPON, res );
    
    if (IS_SET(dam_flag, DAMF_SPELL)) 
        immune_from_flags( ch, IMM_SPELL|IMM_PRAYER|IMM_MAGIC, res );
    
    if (IS_SET(dam_flag, DAMF_PRAYER))
        immune_from_flags( ch, IMM_PRAYER|IMM_SPELL, res );

    if (IS_SET(dam_flag, DAMF_MAGIC))
        immune_from_flags( ch, IMM_MAGIC|IMM_SPELL, res );

    if (IS_SET(dam_flag, DAMF_WATER))
        immune_from_flags( ch, IMM_DROWNING, res );

    immune_from_flags( ch, immune_bit( dam_type ), res );
    
    int result =  immune_resolve( res );
#ifdef DEBUG    
    if (ch->isCoder())
        ch->pecho("damage %s, flags %s, result %s",
                damage_table.name(dam_type).c_str(),
                damage_flags.names(dam_flag).c_str(),
                (res == RESIST_IMMUNE ? "immune" : res == RESIST_RESISTANT ? "resist" : res == RESIST_VULNERABLE ? "vuln" : "normal"));
#endif                
    return result;
}

