/* $Id: practice.cpp,v 1.1.2.15.6.10 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#include <vector>
#include <map>

#include "practice.h"

#include "class.h"

#include "skill.h"
#include "skillmanager.h"
#include "spell.h"
#include "clanreference.h"
#include "pcharacter.h"
#include "room.h"

#include "attract.h"
#include "occupations.h"
#include "comm.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"
#include "act.h"
#include "def.h"

CLAN(battlerager);

COMMAND(CPractice, "practice")
{
    DLString argument = constArguments;

    if (ch->is_npc( ))
	return;
	
    if (argument.empty( ) || arg_is_all( argument ))
	pracShow( ch->getPC( ), true, false );
    else if (arg_oneof_strict( argument, "now", "сейчас" ))
	pracShow( ch->getPC( ), true, true );
    else if (arg_oneof_strict( argument, "here", "здесь" ))
        pracHere( ch->getPC( ) );
    else
	pracLearn( ch->getPC( ), argument );
}

struct PracInfo {
    int percent;
    int weight;
    bool forgetting;
    int adept;
    int parentAdept;
    int maximum;
    DLString name;

    inline const char * getNameColor( ) const
    {
	if (forgetting)
	    return "{r";
	else
	    return "{x";
    }
    inline const char * getPercentColor( ) const
    {
	if (percent == 1)
	    return "{R";
	else if (percent >= maximum)
	    return "{C";
	else if (percent >= adept + parentAdept)
	    return "{c";
	else if (percent > adept)
	    return "{m";
	else
	    return "{x";
    }
};

typedef std::vector<PracInfo> PracInfoList;
typedef std::map<DLString, PracInfoList> PracCategoryMap;


void CPractice::pracShow( PCharacter *ch, bool fAll, bool fUsableOnly )
{
    std::basic_ostringstream<char> buf;
    PracCategoryMap cmap;
    PracCategoryMap::iterator cm_iter;
    PracInfo info;
    DLString category;
    int sp;
    bool fRussian = ch->getConfig( )->ruskills;
    
    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
	Skill *skill = SkillManager::getThis( )->find( sn );
	category = skill->getCategory( );

	if (!skill->available( ch ))
	    continue;
	
	if (!skill->usable( ch, false ) && fUsableOnly)
	    continue;

	if (skill->getSpell( ) 
		&& skill->getSpell( )->isCasted( ) 
		&& ch->getClan( ) == clan_battlerager)
	    continue;

	PCSkillData &data = ch->getSkillData( sn );
	
	if (data.learned <= 1 && !fAll)
	    continue;
	
	info.percent = data.learned;
	info.weight = skill->getWeight( ch );
	info.forgetting = data.forgetting;
	info.name = skill->getNameFor( ch );
	info.adept = skill->getAdept( ch );
	info.parentAdept = ch->getProfession( )->getParentAdept( );
	info.maximum = skill->getMaximum( ch );

	cm_iter = cmap.find( category );

	if (cm_iter == cmap.end( )) {
	    PracInfoList list;

	    list.push_back( info );
	    cmap[category] = list;
	    
	} else
	    cm_iter->second.push_back( info );
    }

    const char *pattern = (fRussian ? "%s%-27s %s%3d%%{x" : "%s%-16s%s%3d%%{x");
    const int columns = (fRussian ? 2 : 3);

    for (cm_iter = cmap.begin( ); cm_iter != cmap.end( ); cm_iter++) {
	unsigned int i;
	
	category = cm_iter->first;
	category.upperFirstCharacter( );
	buf << category << ":" << endl;
	
	for (i = 0; i < cm_iter->second.size( ); i++) {
	    info = cm_iter->second[i];
	     
	    buf << dlprintf( pattern,
	                      info.getNameColor( ), 
	                      info.name.c_str( ),
			      info.getPercentColor( ),
			      info.percent );    	        	    
#if 0	    
	    if (info.weight > 0)
		buf << "*{c" << info.weight / 10 << "." << info.weight % 10 << "{x ";
	    else
#endif                
		buf << "     ";
	    
	    if ((i + 1) % columns == 0)
		buf << endl;
	}

	if (i % columns)
	    buf << endl;
	    
	buf << endl;
    }
    
    buf << dlprintf( "У тебя %d сесси%s практики (practice).\n\r",
                 ch->practice.getValue( ), GET_COUNT(ch->practice, "я","и","й") );
#if 0    
    if ((sp = ch->skill_points( )) > 0)
	buf << "{cУ тебя " << sp << "/" << ch->max_skill_points << " skill points.{x" << endl;
#endif

    page_to_char( buf.str( ).c_str( ), ch );
}

void CPractice::pracHere( PCharacter *ch )
{
    std::basic_ostringstream<char> buf;
    Character *teacher;
    bool found = false;

    if (!( teacher = findTeacher( ch ) ))
	if (!( teacher = findPracticer( ch ) )) {
            ch->println("Тебе не с кем практиковаться здесь.");
	    return;
        }

    buf << fmt( ch, "%1$^C1 может научить тебя таким навыкам:", teacher ) << endl;
    for (int sn = 0; sn < SkillManager::getThis( )->size( ); sn++) {
        ostringstream errbuf;
	Skill *skill = SkillManager::getThis( )->find( sn );

	if (!skill->available( ch ) || !skill->canPractice( ch, errbuf ))
	    continue;

        if (ch->getSkillData( sn ).learned >= skill->getAdept( ch ))
            continue;

        if (teacher->is_npc( ) && !skill->canTeach( teacher->getNPC( ), ch, false ))
            continue;

        if (!teacher->is_npc( ) && skill->getEffective( teacher ) < 100)
            continue;

        DLString sname = skill->getNameFor( ch ).c_str( );
        buf << "     " << sname << endl;
        found = true;
    }
    
    if (found) 
        page_to_char( buf.str( ).c_str( ), ch );
    else
        ch->pecho("Тебе нечему научиться у %C2.", teacher );
}

void CPractice::pracLearn( PCharacter *ch, DLString &arg )
{
    int sn, adept;
    Skill *skill;
    Character *teacher;
    const char * sname;
    ostringstream buf;

    if (!IS_AWAKE( ch )) {
	ch->send_to( "Во сне или как?\n\r");
	return;
    }

    sn = skillManager->unstrictLookup( arg, ch );
    skill = skillManager->find( sn );

    if (!skill) {
	ch->send_to( "Ты не можешь практиковать это.\n\r");
	return;
    }

    if (!skill->canPractice( ch, buf )) {
	ch->send_to( buf );
	return;
    }
    
    if (!( teacher = findTeacher( ch, skill ) ))
	if (!( teacher = findPracticer( ch, skill ) ))
	    return;

    if (ch->practice <= 0) {
	ch->send_to( "У тебя нет сессий практики (practice).\n\r");
	return;
    }

    int &learned = ch->getSkillData( skill->getIndex( ) ).learned;

    adept = skill->getAdept( ch );
    sname = skill->getNameFor( ch ).c_str( );

    if (learned >= adept) {
	ch->printf( "Ты уже знаешь искусство '%s'.\r\n", sname );
	return;
    }

    ch->practice--;
    
    skill->practice( ch );

    act("$c1 обучает тебя искусству '$t'.", teacher, sname, ch, TO_VICT);
    act("Ты обучаешь $C4 искусству '$t'.", teacher, sname, ch, TO_CHAR);
    act("$c1 обучает $C4 искусству '$t'.", teacher, sname, ch, TO_NOTVICT);
    
    if (learned < adept)
	ch->printf( "Ты теперь знаешь '%s' на %d процентов.\n\r", sname, learned );
    else {
	act_p("Ты теперь знаешь '$t'.",ch, sname, 0, TO_CHAR, POS_RESTING);
	act_p("$c1 теперь знает '$t'.",ch, sname, 0, TO_ROOM, POS_RESTING);
    }
}

PCharacter * CPractice::findTeacher( PCharacter *ch, Skill *skill )
{
    Character *rch;
    PCharacter *teacher;
    
    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
	if (rch->is_npc( ))
	    continue;
	
	teacher = rch->getPC( );
	
	if (teacher->getRealLevel( ) < HERO - 1)
	    continue;

	if (!teacher->getAttributes( ).isAvailable( "teacher" ))
	    continue;
	
	if (skill && skill->getEffective( teacher ) < 100)
	    continue;

	return teacher;
    }
    
    return NULL;
}

NPCharacter * CPractice::findPracticer( PCharacter *ch, Skill *skill )
{
    NPCharacter *mob;

    mob = find_attracted_mob( ch, OCC_PRACTICER );

    if (!skill || skill->canTeach( mob, ch )) 
	return mob;
    else
	return NULL;
}

