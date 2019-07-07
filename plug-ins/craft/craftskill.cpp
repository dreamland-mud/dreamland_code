#include "craftskill.h"
#include "craft_utils.h"
#include "subprofession.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "grammar_entities_impl.h"
#include "stringlist.h"
#include "skill_utils.h"
#include "skillmanager.h"                                                       
#include "skillgroup.h"                                                       
#include "skillreference.h"
#include "behavior_utils.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "stats_apply.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

GSN(learning);
BONUS(learning);

const DLString CraftSkill::CATEGORY = "Умения дополнительных профессий";

CraftSkill::CraftSkill( )
{
}

SkillGroupReference &CraftSkill::getGroup( )
{
    return group;
}

bool CraftSkill::visible( Character *ch ) const
{
    CraftProfessions::const_iterator sp;
    XMLAttributeCraft::Pointer attr = craft_attr(ch);

    if (!attr) 
        return false;

    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        const DLString &profName = sp->first;
        if (attr->learned(profName))
            return true;
    }

    return false;
}

bool CraftSkill::available( Character *ch ) const
{
    CraftProfessions::const_iterator sp;
    XMLAttributeCraft::Pointer attr = craft_attr(ch);

    if (!attr)  
        return false;

    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        const DLString &profName = sp->first;
        const int minLevel = sp->second.level;
      
        if (attr->proficiencyLevel(profName) >= minLevel) 
            return true;
    }

    return false;
}

bool CraftSkill::usable( Character *ch, bool verbose ) const
{
    return available(ch);
} 

int CraftSkill::getLevel( Character *ch ) const
{
    return 1;
}

int CraftSkill::getLearned( Character *ch ) const
{
    if (!usable( ch, false ))
        return 0;

    return ch->getPC( )->getSkillData( getIndex( ) ).learned;
}

int CraftSkill::getWeight( Character *ch ) const
{
    return weight;
}

bool CraftSkill::canForget( PCharacter *ch ) const
{
    return false;
}

bool CraftSkill::canPractice( PCharacter *ch, std::ostream &buf ) const
{
    return available(ch);
}

bool CraftSkill::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose )
{
    if (!mob) {
        if (verbose)
            ch->println( "Тебе не с кем практиковаться здесь." );
        return false;
    }
    
    if (mob->pIndexData->practicer.isSet( (int)getGroup( ) ))
        return true;

    if (verbose)
        ch->pecho( "%1$^C1 не может научить тебя искусству '%2$s'.\n"
               "Для большей информации используй: {y{hc{lRумение %2$s{lEslook %2$s{x, {y{lRгруппаумен {Dгруппа{y{lEglist {Dгруппа{x.",
               mob, getNameFor( ch ).c_str( ) );
    return false;
}

void CraftSkill::show( PCharacter *ch, std::ostream &buf )
{
    buf << skill_what(this).ruscase('1').upperFirstCharacter() 
        << " '{c" << getName( ) << "{x' или"
        << " '{c" << getRussianName( ) << "{x', "
        << "входит в группу '{hg{c" << getGroup()->getNameFor(ch) << "{x'." 
        << endl << endl;

    StringList pnames; 
    CraftProfessions::const_iterator sp;
    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(sp->first);
        if (prof) 
            pnames.push_back(prof->getNameFor(ch));
    }

    if (!pnames.empty())
        buf << "Доступно профессии " << pnames.wrap("{W", "{x").join(", ") << "." << endl;

    print_see_also(this, ch, buf);
} 

static void mprog_skill( Character *ch, Character *actor, const char *skill, bool success, Character *victim )
{
    FENIA_VOID_CALL( ch, "Skill", "CsiC", actor, skill, success, victim );
    FENIA_NDX_VOID_CALL( ch->getNPC( ), "Skill", "CCsiC", ch, actor, skill, success, victim );
}

static void rprog_skill( Room *room, Character *actor, const char *skill, bool success, Character *victim )
{
    FENIA_VOID_CALL( room, "Skill", "CsiC", actor, skill, success, victim );

    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        mprog_skill( rch, actor, skill, success, victim );
}

void CraftSkill::improve( Character *ch, bool success, Character *victim, int dam_type, int dam_flags ) const 
{
    PCharacter *pch;
    int chance, xp;
    
    if (ch->is_npc( ))
        return;

    pch = ch->getPC( );
    PCSkillData &data = pch->getSkillData( getIndex( ) );
    
    if (!usable( pch, false )) 
        return;     

    if (data.learned <= 1)
        return;
    
    rprog_skill( ch->in_room, ch, getName( ).c_str( ), success, victim );

    if (data.forgetting)
        return;

    data.timer = 0;

    if (data.learned >= getMaximum( pch ))
        return;

    chance = 1000;
    chance /= max(1, hard.getValue()) * getRating(pch);
    chance = chance * get_int_app(pch).learn  / 100;
    
    if (bonus_learning->isActive(pch, time_info))
        chance *= 2;

    if (number_range(1, 1000) > chance)
        return;
   
    if (success) {
        chance = URANGE(5, 100 - data.learned, 95);
        
        if (number_percent( ) >= chance)
            return;
            
        act_p("{GТеперь ты гораздо лучше владеешь искусством '$t'!{x",
                pch, getNameFor( pch ).c_str( ), 0, TO_CHAR, POS_DEAD);
            
        data.learned++;
    }
    else {
        int wis_mod = get_wis_app( ch ).learn;
        
        if (wis_mod <= 0)
            return;

        chance = URANGE(5, data.learned / 2, wis_mod * 15);
            
        if (number_percent( ) >= chance)
            return;

        act_p("{GТы учишься на своих ошибках, и твое умение '$t' совершенствуется.{x",
                pch, getNameFor( pch ).c_str( ), 0, TO_CHAR, POS_DEAD);
        
        data.learned += number_range( 1, wis_mod );
        data.learned = min( (int)data.learned, getMaximum( pch ) );
    }

    pch->updateSkills( );
    xp = 2 * getRating( pch );

    if (pch->isAffected(gsn_learning ))
        xp += data.learned / 4;

    if (data.learned >= getMaximum( pch )) {
        act_p("{WТеперь ты {Cмастерски{W владеешь искусством {C$t{W!{x",
              pch, getNameFor( pch ).c_str( ), 0, TO_CHAR, POS_DEAD);
        
        xp += 98 * getRating( pch );
    }
   
     
    CraftProfessions::const_iterator sp;
    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(sp->first);
        if (prof)
            prof->gainExp(pch, xp);
    }
}

