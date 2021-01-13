/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "defaultskillgroup.h"
#include "skill.h"
#include "skillcommand.h"
#include "spell.h"
#include "skillmanager.h"
#include "pcharacter.h"
#include "websocketrpc.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GROUP(clan);

/*-------------------------------------------------------------------
 * DefaultSkillGroup
 *------------------------------------------------------------------*/
DefaultSkillGroup::DefaultSkillGroup( )
            : autoHelp( true )
{
}


void DefaultSkillGroup::loaded( )
{
    skillGroupManager->registrate( Pointer( this ) );

    if (help) 
        help->setSkillGroup( Pointer( this ) );
}

void DefaultSkillGroup::unloaded( )
{
    if (help) 
        help->unsetSkillGroup( );

    skillGroupManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultSkillGroup::getShortDescr( ) const
{
    return shortDescr;
}

const DLString &DefaultSkillGroup::getRussianName( ) const
{
    return nameRus.getFullForm( );
}

int DefaultSkillGroup::getPracticer( ) const
{
    return practicer;
}

bool DefaultSkillGroup::visible( Character *ch ) const
{
    return !hidden;
}

bool DefaultSkillGroup::available( Character *ch ) const
{
    if (ch->is_npc( ) && IS_CHARMED(ch))
        return !nopet;

    return true;
}

void DefaultSkillGroup::show( PCharacter *ch, ostringstream &buf ) const
{
    buf << "Группа "
        << "{Y" << getName( ) << "{w, "
        << "{Y" << getRussianName( ) << "{w ";
        
    if (help)
        buf << web_edit_button(ch, "hedit", help->getID());
        
    buf << endl;

    if (help)
        buf << help->c_str( );

    if (autoHelp) {
        buf << endl;
        listPracticers( ch, buf );
        listSkills( ch, buf );
        buf << endl;
    }
}

void DefaultSkillGroup::listPracticers( PCharacter *ch, ostringstream &buf ) const
{
    if (group_clan == this) {
        buf << "Практикуется у {gкланового охранника{x.";
    }
    else if (practicer == 0) {
        buf << "Практикуется в {gгильдии{x.";
    }
    else {
        MOB_INDEX_DATA *pMob = get_mob_index( practicer );

        if (!pMob) {
            LogStream::sendError( ) << "Group " << getName( ) << " has invalid practicer " << practicer << endl;
            return;
        }

        if (pMob->sex == SEX_FEMALE)
            buf << "Учительница";
        else
            buf << "Учитель";

        buf << " - {g" 
            << russian_case( pMob->short_descr, '1' ) << "{x "
            << "({g{hh" << pMob->area->name << "{x).";
    }

    buf << endl;
}

void DefaultSkillGroup::listSkills( PCharacter *ch, ostringstream &buf ) const
{
    // True if it's a help json dump and not a player requesting the article.
    bool autodump = ch->desc == 0;
    bool fRus = ch->getConfig().ruskills;
    int columns = 3;
    const char *pattern = fRus ? "{%c%-26s{x" : "{%c%-20s{x";
    const char *autopattern = "%-26s";

    buf << endl << "Навыки этой группы:" << endl;

    for (int col = 0, sn = 0; sn < skillManager->size( ); sn++) {
        Skill *skill = skillManager->find( sn );

        if (skill->getGroup( ) != this)
            continue;

        if (autodump) {
            DLString id;
            if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
                id = DLString(skill->getSkillHelp()->getID());

            DLString skillName = skill->getNameFor(ch) + "{hx";
            buf << "{hh" << id << dlprintf(autopattern, skillName.c_str());

        } else {
            buf << fmt(0, pattern, getSkillColor(skill, ch), skill->getNameFor(ch).c_str());
        }
        
        if (++col % columns == 0)
            buf << endl;
    }
}

char DefaultSkillGroup::getSkillColor( Skill *skill, PCharacter *ch ) const
{
    if (skill->usable( ch, false ))
        return 'g';

    return 'w';
}

bool DefaultSkillGroup::matchesUnstrict( const DLString &str ) const
{
    if (!name.empty( ) 
        && str.strPrefix( name ))    
    {
        return true;
    }

    if (!nameRus.getFullForm( ).empty( ) 
        && is_name( str.c_str( ),
                    nameRus.decline( '7' ).c_str( ) ))
    {
        return true;
    }

    return false;
}

