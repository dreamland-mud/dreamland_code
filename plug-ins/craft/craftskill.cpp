#include "craftskill.h"
#include "subprofession.h"

#include "grammar_entities_impl.h"
#include "skillmanager.h"                                                       
#include "skillgroup.h"                                                       
#include "behavior_utils.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

const DLString CraftSkill::CATEGORY = "Умения крафтинга";

CraftSkill::CraftSkill( )
{
}

SkillGroupReference &CraftSkill::getGroup( )
{
    return group;
}

XMLAttributeCraft::Pointer CraftSkill::getAttr(Character *ch) const
{
    XMLAttributeCraft::Pointer attr;
    if (!ch->is_npc())
        attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeCraft>("craft");
    return attr;
}


bool CraftSkill::visible( Character *ch ) const
{
    CraftProfessions::const_iterator sp;
    XMLAttributeCraft::Pointer attr = getAttr(ch);

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
    XMLAttributeCraft::Pointer attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeCraft>("craft");

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
    bool rus = ch->getConfig( )->ruskills;

    buf << (spell && spell->isCasted( ) ? "Заклинание" : "Умение")
        << " '{W" << getName( ) << "{x'"
	<< " '{W" << getRussianName( ) << "{x', "
	<< "входит в группу '{hg{W" 
	<< (rus ? getGroup( )->getRussianName( ) : getGroup( )->getName( )) 
	<< "{x'"
	<< endl;


    DLString pbuf;
    CraftProfessions::const_iterator sp;
    bool found = false;
    for (sp = subprofessions.begin(); sp != subprofessions.end(); sp++) {
	CraftProfession::Pointer prof = craftProfessionManager->get(sp->first);
        if (prof) {
            if (found)
		pbuf << ", ";        
	    pbuf << prof->getNameFor(ch);
            found = true;
       }
    }

    if (!pbuf.empty())
        buf << "Доступно профессии " << pbuf << endl;
} 

