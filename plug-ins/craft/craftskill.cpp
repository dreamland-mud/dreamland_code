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

#include "act.h"
#include "def.h"

GSN(learning);
BONUS(learning);

const DLString CraftSkill::CATEGORY = "Умения дополнительных профессий";

CraftSkill::CraftSkill( )
                : group(skillGroupManager)
{
}

GlobalBitvector & CraftSkill::getGroups( ) 
{
    return group;
}

bool CraftSkill::visible( CharacterMemoryInterface *ch ) const
{
    if (!ch->getPCM())
        return false;

    CraftProfessions::const_iterator sp;
    XMLAttributeCraft::Pointer attr = craft_attr(ch->getPCM());

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

bool CraftSkill::canPractice( PCharacter *ch, std::ostream &buf ) const
{
    return available(ch);
}

bool CraftSkill::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose )
{
    if (!mob) {
        if (verbose)
            ch->pecho( "Тебе не с кем практиковаться здесь." );
        return false;
    }
    
    if (mob->pIndexData->practicer.isSetAny(getGroups()))
        return true;

    if (verbose)
        ch->pecho( "%1$^C1 не может научить тебя искусству '%2$s'.\n"
               "Учителя можно найти, прочитав справку по этому умению.", 
               mob, getNameFor( ch ).c_str( ) );
    return false;
}

void CraftSkill::show( PCharacter *ch, std::ostream &buf ) const
{
    buf << print_what(this) << " "
        << print_names_for(this, ch)
        << print_group_for(this, ch)
        << ".{x" << endl;

    StringList pnames; 
    CraftProfessions::const_iterator sp;
    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(sp->first);
        if (prof) 
            pnames.push_back(prof->getNameFor(ch));
    }

    if (!pnames.empty())
        buf << SKILL_INFO_PAD << "Доступно профессии " << pnames.wrap("{W", "{x").join(", ") << "." << endl;
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
    const Skill *thiz = static_cast<const Skill *>(this);
    
    if (ch->is_npc( ))
        return;

    pch = ch->getPC( );
    PCSkillData &data = pch->getSkillData( getIndex( ) );
    
    if (!usable( pch, false )) 
        return;     

    if (data.learned <= 1)
        return;
    
    rprog_skill( ch->in_room, ch, getName( ).c_str( ), success, victim );

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
            
        pch->pecho("{GТеперь ты гораздо лучше владеешь искусством '%K'!{x", thiz);

        data.learned++;
    }
    else {
        int wis_mod = get_wis_app( ch ).learn;
        
        if (wis_mod <= 0)
            return;

        chance = URANGE(5, data.learned / 2, wis_mod * 15);
            
        if (number_percent( ) >= chance)
            return;

        pch->pecho("{GТы учишься на своих ошибках, и твое умение '%K' совершенствуется.{x", thiz);
        
        data.learned += number_range( 1, wis_mod );
        data.learned = min( (int)data.learned, getMaximum( pch ) );
    }

    pch->updateSkills( );
    xp = 2 * getRating( pch );

    if (pch->isAffected(gsn_learning ))
        xp += data.learned / 4;

    if (data.learned >= getMaximum( pch )) {
        pch->pecho("{WТеперь ты {Cмастерски{W владеешь искусством {C%K{W!{x", thiz);
        
        xp += 98 * getRating( pch );
    }
   
     
    CraftProfessions::const_iterator sp;
    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
        CraftProfession::Pointer prof = craftProfessionManager->get(sp->first);
        if (prof)
            prof->gainExp(pch, xp);
    }
}

