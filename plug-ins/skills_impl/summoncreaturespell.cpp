/* $Id$
 *
 * ruffina, 2004
 */
#include "summoncreaturespell.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "affect.h"

#include "interp.h"
#include "save.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "def.h"

int SummonCreatureSpell::countMobiles( Character *ch ) const
{
    Character *wch;
    int count;

    for (wch = char_list, count = 0; wch; wch = wch->next)
        if (wch->is_npc( )
              && IS_CHARMED(wch)
              && wch->master == ch
              && wch->getNPC( )->pIndexData->vnum == mobVnum.getValue( ))
            count++;

    return count;
}

NPCharacter * SummonCreatureSpell::createMobileAux( 
                Character *ch, int level, int hit, int mana,
                int dice_number, int dice_type, int dice_bonus ) const
{
    int i;
    NPCharacter *mob = create_mobile( get_mob_index( mobVnum.getValue( ) ) );

    mob->setLevel( max( 1, ch->applyCurse( level ) ) );

    for (i = 0; i < stat_table.size; i++)
        mob->perm_stat[i] = ch->applyCurse( ch->getCurrStat( i ) ); 

    mob->hit = mob->max_hit = URANGE( 
                                   ch->applyCurse( ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit ), 
                                   ch->applyCurse( hit ), 
                                   30000 );
    mob->mana = mob->max_mana = ch->applyCurse( mana );
    
    mob->armor[3] = interpolate( level, 100, 0 );
    for (i = 0; i < 3; i++)
        mob->armor[i] = interpolate( level, 100, -100 );
    
    if (IS_SET( mob->act, ACT_NOALIGN ))
        mob->alignment = ch->alignment;

    if (mob->pIndexData->sex == SEX_EITHER)
        mob->setSex( ch->getSex( ) );

    mob->gold = 0;
    mob->damage[DICE_NUMBER] = dice_number;
    mob->damage[DICE_TYPE] = dice_type;
    mob->damage[DICE_BONUS] = dice_bonus;

    // Make sure all permanent affect on the mob have non-zero level.
    for (Affect *paf = mob->affected; paf; paf = paf->next) {
        if (paf->duration == -1)
            paf->level = mob->getRealLevel();
    }

    return mob;
}

void SummonCreatureSpell::run( Character *ch, char *, int sn, int level )
{
    int cnt;
    
    if (ch->is_npc( ))
        return;

    if (ch->isAffected( sn )) {
        ch->pecho( msgStillAffected.getValue( ).c_str( ) );
        return;
    }
    
    if (!msgCreateAttemptArea.getValue( ).empty( ))
        interpret_raw( ch, "yell", msgCreateAttemptArea.getValue( ).c_str( ) );
    else {
        ch->pecho( msgCreateAttemptSelf.getValue( ).c_str( ) );
        ch->recho( POS_RESTING, msgCreateAttemptRoom.getValue( ).c_str( ), ch );
    }
    
    cnt = countMobiles( ch );
    cnt = min( castMobCount.getValue( ), maxMobCount.getValue( ) - cnt );
    
    if (cnt <= 0) {
        ch->pecho( msgTooManyMobiles.getValue( ).c_str( ) );
        return;
    }

    if (!canSummonHere( ch )) 
        return;

    postaffect_to_char( ch, sn, postaffectDuration );

    for (int i = 0; i < cnt; i++) { 
        SummonedCreature::Pointer bhv;
        NPCharacter *mob = createMobile( ch, level );

        SET_BIT( mob->affected_by, AFF_CHARM );
        mob->master = mob->leader = ch;
        
        bhv = mob->behavior.getDynamicPointer<SummonedCreature>( );
        bhv->creatorName = ch->getName( );
        bhv->creatorID = ch->getID( );

        char_to_room( mob, ch->in_room );
        bhv->save( );
        bhv->conjure( );
    }

    // Show different message when only one lion/panther/guard out of 2 appears.
    if (cnt == 1 && cnt != castMobCount) {
        ch->pecho( msgCreateSelfOne.getValue( ).c_str( ) );
        ch->recho( POS_RESTING, msgCreateRoomOne.getValue( ).c_str( ), ch );
    } else {
        ch->pecho( msgCreateSelf.getValue( ).c_str( ) );
        ch->recho( POS_RESTING, msgCreateRoom.getValue( ).c_str( ), ch );
    }
}

void SummonCreatureSpell::run( Character *ch, Character *victim, int sn, int level )
{
    SummonedCreature::Pointer bhv;
    const char * shortDescr = get_mob_index( mobVnum )->short_descr;
    
    if (ch->is_npc( ))
        return;

    if (!victim->is_npc( )) {
        ch->pecho( "%^C3 вряд ли понравится быть %N5.", victim, shortDescr);
        return;
    }
    
    if (victim->getNPC( )->pIndexData->vnum != mobVnum) {
        ch->pecho( "%1$^C1 разве похо%1$Gж|ж|жа на %2$N4?", victim, shortDescr);
        return;
    }
    
    if (IS_CHARMED(victim)) {
        ch->pecho( "У %C2 уже есть хозяин.", victim );
        return;
    }
 
    ch->pecho( msgReattachAttemptSelf.getValue( ).c_str( ), victim );
    ch->recho( POS_RESTING, msgReattachAttemptRoom.getValue( ).c_str( ), ch, victim );

    if (!victim->getNPC( )->behavior
            || !( bhv = victim->getNPC( )->behavior.getDynamicPointer<SummonedCreature>( ) )
            || bhv->creatorName.getValue( ) != ch->getName( )
            || bhv->creatorID.getValue( ) != ch->getID( ))
    {
        ch->recho( POS_RESTING, msgNotRecognizeRoom.getValue( ).c_str( ), ch, victim );
        ch->pecho( msgNotRecognizeSelf.getValue( ).c_str( ), ch, victim );
        return;
    }
    
    if (countMobiles( ch ) >= maxMobCount) {
        ch->pecho( msgTooManyMobiles.getValue( ).c_str( ) );
        return;
    }
    
    SET_BIT( victim->affected_by, AFF_CHARM );
    victim->leader = victim->master = ch;
    bhv->save( );
    save_mobs( ch->in_room );

    ch->recho( POS_RESTING, msgReattachRoom.getValue( ).c_str( ), ch, victim );
    ch->pecho( msgReattachSelf.getValue( ).c_str( ), victim );
}


void SummonedCreature::conjure( )
{
}

