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

int immune_check(Character *ch, int dam_type, bitstring_t dam_flag)
{
    bitstring_t bit;
    int res;

    res = RESIST_NORMAL;

    if (dam_type == DAM_NONE)
        return res;
    
    if (IS_SET(dam_flag, DAMF_WEAPON)) 
        immune_from_flags( ch, IMM_WEAPON, res );
    
    if (IS_SET(dam_flag, DAMF_SPELL)) 
        immune_from_flags( ch, IMM_SPELL, res );
    
    if (IS_SET(dam_flag, DAMF_PRAYER))
        immune_from_flags( ch, IMM_PRAYER, res );

    if (IS_SET(dam_flag, DAMF_MAGIC))
        immune_from_flags( ch, IMM_MAGIC, res );

    /* set bits to check -- VULN etc. must ALL be the same or this will fail */
    switch (dam_type)
    {
        case(DAM_BASH):                bit = IMM_BASH;                break;
        case(DAM_PIERCE):        bit = IMM_PIERCE;        break;
        case(DAM_SLASH):        bit = IMM_SLASH;        break;
        case(DAM_FIRE):                bit = IMM_FIRE;                break;
        case(DAM_COLD):                bit = IMM_COLD;                break;
        case(DAM_LIGHTNING):        bit = IMM_LIGHTNING;        break;
        case(DAM_ACID):                bit = IMM_ACID;                break;
        case(DAM_POISON):        bit = IMM_POISON;        break;
        case(DAM_NEGATIVE):        bit = IMM_NEGATIVE;        break;
        case(DAM_HOLY):                bit = IMM_HOLY;                break;
        case(DAM_ENERGY):        bit = IMM_ENERGY;        break;
        case(DAM_MENTAL):        bit = IMM_MENTAL;        break;
        case(DAM_DISEASE):        bit = IMM_DISEASE;        break;
        case(DAM_DROWNING):        bit = IMM_DROWNING;        break;
        case(DAM_LIGHT):        bit = IMM_LIGHT;        break;
        case(DAM_CHARM):        bit = IMM_CHARM;        break;
        case(DAM_SOUND):        bit = IMM_SOUND;        break;
        default:                bit = 0;                break;
    }
    
    immune_from_flags( ch, bit, res );
    
    return immune_resolve( res );
}

