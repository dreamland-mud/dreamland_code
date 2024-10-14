#include "player_exp.h"

/* $Id$
 *
 * ruffina, 2004
 */
#include "clanreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "pcrace.h"
#include "skill.h"
#include "skillmanager.h"
#include "room.h"
#include "commonattributes.h"
#include "hometown.h"
#include "wiznet.h"
#include "infonet.h"
#include "messengers.h"
#include "act.h"
#include "stats_apply.h"

#include "merc.h"
#include "def.h"

CLAN(none);
PROF(samurai);
HOMETOWN(frigate);

 // Count total number of skills available at this level.
static int count_available_skills(PCharacter *ch)
{
    int availCounter = 0;

    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        PCSkillData &data = ch->getSkillData(sn);

        if (data.origin == SKILL_PRACTICE && skill->available(ch))
            availCounter++;
    }

    return availCounter;
}


/*
 *  Gain experience
 */
void Player::gainExp(PCharacter *pch, int gain)
{
    int level = pch->getLevel();

    if (level >= LEVEL_HERO - 1)
        return;

    if (level > 19  && !IS_SET( pch->act, PLR_CONFIRMED )) {
        pch->pecho("Ты больше не можешь получать опыт, пока тебя не подтвердили Боги.\n\r"
                "Прочитай 'справка подтверждение'.");
        return;
    }

    if (level > 19  && pch->getHometown( ) == home_frigate ) {
        pch->pecho("Ты больше не можешь получать опыт, пока не выберешь дом.\n\r"
                "Прочитай 'справка родной город'.");
        return;
    }

    if (level >= PK_MIN_LEVEL && IS_SET(pch->in_room->room_flags, ROOM_NEWBIES_ONLY)) {
        pch->pecho("Ты не можешь больше получать опыт в этой арии.");
        return;
    }

    if (IS_SET(pch->act,PLR_NO_EXP)) {
        pch->pecho("Ты не можешь получать опыт, пока твой дух во власти противника.");
        return;
    }
    
    if (pch->getAttributes().isAvailable( "noexp" ))
        return;

    pch->exp = max( pch->getExpPerLevel( level ), pch->exp + gain ); 

    while (pch->getLevel() < LEVEL_HERO - 1 && pch->getExpToLevel( ) <= 0) {
        
        oldact_p("{CТы дости$gгло|г|гла следующего уровня!!!{x", pch, 0, 0, TO_CHAR, POS_DEAD);

        int lastLevelSkillCount = count_available_skills(pch);

        pch->setLevel( pch->getLevel() + 1 );

        /* added for samurais by chronos */
        if (pch->getProfession( ) == prof_samurai && level == 10)
            pch->wimpy = 0;

        infonet(pch, 0, "{CРадостный голос из $o2: ", "{W%1$#C1 дости%1$G#гло|г|гла следующей ступени мастерства.{x", pch);

        ::wiznet( WIZ_LEVELS, 0, 0, 
                  "%1$^C1 дости%1$Gгло|г|гла %2$d уровня!", pch, pch->getLevel( ) );

        send_discord_level(pch);
        send_telegram_level(pch);

        advanceLevel(pch);

        // Notify about any new skills gained due to the level advancement.
        int thisLevelSkillCount = count_available_skills(pch);
        int skillsDiff = thisLevelSkillCount - lastLevelSkillCount;
        if (skillsDiff > 0)
            pch->pecho("{CТебе открыл%1$Iось|ись|ись {Y%1$d{C нов%1$Iое|ых|ых умени%1$Iе|я|й.{x", skillsDiff);

        pch->save( );
    }
}

/*
 * Advancement through levels.
 */
void Player::advanceLevel(PCharacter *pch)
{
    ostringstream buf;
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
    int add_train;

    pch->last_level = pch->age.getTrueHours( );
    
    add_hp = (pch->getCurrStat(STAT_CON) * pch->getProfession( )->getHpRate( )) / 100;

    add_mana = number_range(pch->getCurrStat(STAT_INT)/2 + 10,
            2 * pch->getCurrStat(STAT_INT) + pch->getCurrStat(STAT_WIS)/5 - 10);

    add_mana = (add_mana * pch->getProfession( )->getManaRate( )) / 100;

    add_move = number_range( 1, (pch->getCurrStat(STAT_CON) + pch->getCurrStat(STAT_DEX))/6 );

    add_prac = get_wis_app( pch ).practice;

    add_hp = max( 3, add_hp );
    add_mana = max( 3, add_mana );
    add_move = max( 6, add_move );
    add_train = pch->getLevel( ) % 5 == 0 ? 1 : 0;

    add_hp += pch->getRemorts().getHitPerLevel( pch->getLevel() );
    add_mana += pch->getRemorts().getManaPerLevel( pch->getLevel() ); 

    pch->max_hit         += add_hp;
    pch->max_mana        += add_mana;
    pch->max_move        += add_move;
    pch->practice        += add_prac;
    pch->train       += add_train;

    pch->perm_hit        += add_hp;
    pch->perm_mana        += add_mana;
    pch->perm_move        += add_move;
    
    buf << "{CТы получаешь: "
        << "{Y" << add_hp << "{C/" << pch->max_hit << " здоровья, "
        << "{Y" << add_mana << "{C/" << pch->max_mana << " маны, "
        << "{Y" << add_move << "{C/" << pch->max_move << " движения, "
        <<  endl <<  "              " 
        << "{Y" << add_prac << "{C/" << pch->practice << " практики";

    if (add_train > 0)
        buf << ", {Y" << add_train << "{C/" << pch->train << " тренировку";
    
    buf << ".{x";
    pch->pecho( buf.str( ).c_str( ) );

    pch->updateSkills( );
}
