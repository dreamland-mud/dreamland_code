/* $Id$
 *
 * ruffina, 2004
 */
#include "mobileprofession.h"    
#include "profflags.h"

#include "npcharacter.h"
#include "merc.h"
#include "def.h"
    
PROF(vampire);
PROF(warrior);
PROF(necromancer);
PROF(cleric);
PROF(warlock);
PROF(witch);
PROF(thief);
PROF(ranger);

int  MobileProfession::getThac32( Character *ch ) const
{
    if (!ch)
        return DefaultProfession::getThac32( ch );

    checkTarget( ch );

    if (IS_SET(ch->act, ACT_WARRIOR))
        return -10;
    else if (IS_SET(ch->act, ACT_THIEF))
        return -4;
    else if (IS_SET(ch->act, ACT_CLERIC))
        return 2;
    else if (IS_SET(ch->act, ACT_MAGE))
        return 6;
    else
        return DefaultProfession::getThac32( ch );
}

Flags MobileProfession::getFlags( Character *ch ) const
{
    Flags flags( 0, &prof_flags );
    
    if (!ch)
        return flags;

    checkTarget( ch );
    
    GlobalBitvector profs = toVector(ch);
    for (int p = 0; p < professionManager->size(); p++)
        if (profs.isSet(p))
            flags.setBit(professionManager->find(p)->getFlags());

    return flags;
}

int MobileProfession::getStat( bitnumber_t s, Character *ch ) const
{
    EnumerationArray bonuses( &stat_table );
    
    if (!ch)
        return 0;

    checkTarget( ch );

    if (IS_SET(ch->act,ACT_WARRIOR)) {
        bonuses[STAT_STR] += 3;
        bonuses[STAT_INT] -= 1;
        bonuses[STAT_CON] += 2;
    }

    if (IS_SET(ch->act,ACT_THIEF)) {
        bonuses[STAT_DEX] += 3;
        bonuses[STAT_INT] += 1;
        bonuses[STAT_WIS] -= 1;
    }

    if (IS_SET(ch->act,ACT_CLERIC)) {
        bonuses[STAT_WIS] += 3;
        bonuses[STAT_DEX] -= 1;
        bonuses[STAT_STR] += 1;
    }

    if (IS_SET(ch->act,ACT_MAGE)) {
        bonuses[STAT_INT] += 3;
        bonuses[STAT_STR] -= 1;
        bonuses[STAT_DEX] += 1;
    }
    
    return bonuses[s];
}

bool MobileProfession::isPlayed( ) const
{
    return false;
}

GlobalBitvector MobileProfession::toVector( CharacterMemoryInterface *mem ) const
{
    GlobalBitvector bv = DefaultProfession::toVector( mem );

    if (!mem)
        return bv;

    checkTarget( mem );
    
    if (mem->getMobile()) {
        NPCharacter *ch = mem->getMobile();
        
        if (IS_SET(ch->act, ACT_NECROMANCER))  bv.set( prof_necromancer );
        if (IS_SET(ch->act, ACT_UNDEAD))       bv.set( prof_necromancer );
        if (IS_SET(ch->act, ACT_WARRIOR))      bv.set( prof_warrior );
        if (IS_SET(ch->act, ACT_THIEF))        bv.set( prof_thief );
        if (IS_SET(ch->act, ACT_CLERIC))       bv.set( prof_cleric );
        if (IS_SET(ch->act, ACT_MAGE))         bv.set( prof_warlock );
        if (IS_SET(ch->act, ACT_MAGE))         bv.set( prof_witch );
        if (IS_SET(ch->act, ACT_VAMPIRE))      bv.set( prof_vampire );
        if (IS_SET(ch->act, ACT_RANGER))       bv.set( prof_ranger );
    }

    return bv;
}

void MobileProfession::checkTarget( CharacterMemoryInterface *ch ) const 
{
    if (ch->getPCM())
        throw Exception( ch->getName( ) + " has mobile profession!" );
}

