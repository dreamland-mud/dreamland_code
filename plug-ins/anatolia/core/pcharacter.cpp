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
 *  Experience
 */
void PCharacter::gainExp( int gain )
{
    if (level >= LEVEL_HERO - 1)
        return;

    if (level > 19  && !IS_SET( act, PLR_CONFIRMED )) {
        pecho("Ты больше не можешь получать опыт, пока тебя не подтвердили Боги.\n\r"
                "Прочитай 'справка подтверждение'.");
        return;
    }

    if (level > 19  && this->getPC( )->getHometown( ) == home_frigate ) {
        pecho("Ты больше не можешь получать опыт, пока не выберешь дом.\n\r"
                "Прочитай 'справка родной город'.");
        return;
    }

    if (level >= PK_MIN_LEVEL && IS_SET(in_room->room_flags, ROOM_NEWBIES_ONLY)) {
        pecho("Ты не можешь больше получать опыт в этой арии.");
        return;
    }

    if (IS_SET(act,PLR_NO_EXP)) {
        pecho("Ты не можешь получать опыт, пока твой дух во власти противника.");
        return;
    }
    
    if (attributes.isAvailable( "noexp" ))
        return;

    exp = max( getExpPerLevel( level ), exp + gain ); 

    while (level < LEVEL_HERO - 1 && getExpToLevel( ) <= 0) {
        
        oldact_p("{CТы дости$gгло|г|гла следующего уровня!!!{x", this, 0, 0, TO_CHAR, POS_DEAD);

        int lastLevelSkillCount = count_available_skills(this);

        setLevel( level + 1 );

        /* added for samurais by chronos */
        if (getProfession( ) == prof_samurai && level == 10)
            wimpy = 0;

        infonet(this, 0, "{CРадостный голос из $o2: ", "{W%1$#C1 дости%1$G#гло|г|гла следующей ступени мастерства.{x", this);

        ::wiznet( WIZ_LEVELS, 0, 0, 
                  "%1$^C1 дости%1$Gгло|г|гла %2$d уровня!", this, getRealLevel( ) );

        send_discord_level(this);
        send_telegram_level(this);

        advanceLevel( );

        // Notify about any new skills gained due to the level advancement.
        int thisLevelSkillCount = count_available_skills(this);
        int skillsDiff = thisLevelSkillCount - lastLevelSkillCount;
        if (skillsDiff > 0)
            pecho("{CТебе открыл%1$Iось|ись|ись {Y%1$d{C нов%1$Iое|ых|ых умени%1$Iе|я|й.{x", skillsDiff);

        save( );
    }
}

/*
 * Advancement stuff.
 */
void PCharacter::advanceLevel( )
{
    ostringstream buf;
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;
    int add_train;

    last_level = age.getTrueHours( );
    
    add_hp = (getCurrStat(STAT_CON) * getProfession( )->getHpRate( )) / 100;

    add_mana = number_range(getCurrStat(STAT_INT)/2 + 10,
            2 * getCurrStat(STAT_INT) + getCurrStat(STAT_WIS)/5 - 10);

    add_mana = (add_mana * getProfession( )->getManaRate( )) / 100;

    add_move = number_range( 1, (getCurrStat(STAT_CON) + getCurrStat(STAT_DEX))/6 );

    add_prac = get_wis_app( this ).practice;

    add_hp = max( 3, add_hp );
    add_mana = max( 3, add_mana );
    add_move = max( 6, add_move );
    add_train = getRealLevel( ) % 5 == 0 ? 1 : 0;

    add_hp += remorts.getHitPerLevel( level );
    add_mana += remorts.getManaPerLevel( level ); 

    max_hit         += add_hp;
    max_mana        += add_mana;
    max_move        += add_move;
    practice        += add_prac;
    train       += add_train;

    perm_hit        += add_hp;
    perm_mana        += add_mana;
    perm_move        += add_move;
    
    buf << "{CТы получаешь: "
        << "{Y" << add_hp << "{C/" << max_hit << " здоровья, "
        << "{Y" << add_mana << "{C/" << max_mana << " маны, "
        << "{Y" << add_move << "{C/" << max_move << " движения, "
        <<  endl <<  "              " 
        << "{Y" << add_prac << "{C/" << practice << " практики";

    if (add_train > 0)
        buf << ", {Y" << add_train << "{C/" << train << " тренировку";
    
    buf << ".{x";
    pecho( buf.str( ).c_str( ) );

    updateSkills( );
}

