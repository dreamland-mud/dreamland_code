/* $Id: showskill.cpp,v 1.1.2.4.10.1 2008/02/23 13:41:24 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "skill.h"
#include "skillmanager.h"
#include "pcharacter.h"
#include "interp.h"
#include "skillgroup.h"

GROUP(none);

static void print_see_also(Skill *skill, PCharacter *ch, ostream &buf) 
{
    // 'См. также справка|help травы|herbs' - с гипер-ссылкой на справку.
    buf << endl << "См. также {W{lRсправка{lEhelp{lx {hh" << skill->getNameFor(ch) << "{x." << endl;
}
  

CMDRUN( showskill )
{
    std::basic_ostringstream<char> buf;
    int sn;
    Skill *skill;
    DLString argument( constArguments );
    PCharacter *pch = ch->getPC( );
    
    if (ch->is_npc( ))
        return;
    
    if (argument.empty( )) {
        ch->send_to( "Использование: {y{lRумение{lEslook{x <умение или заклинание>\r\n" );
        return;
    }

    sn = SkillManager::getThis( )->unstrictLookup( argument );
    skill = SkillManager::getThis( )->find( sn );
    
    if (!skill) {
        ch->send_to( "Такого умения нет.\r\n" );
        return;
    }
    
    skill->show( pch, buf );
    print_see_also(skill, pch, buf);
    ch->send_to( buf );
}


