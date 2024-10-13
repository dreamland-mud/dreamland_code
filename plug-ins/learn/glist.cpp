/* $Id: glist.cpp,v 1.1.2.2.10.8 2009/09/05 18:30:47 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillmanager.h"
#include "helpmanager.h"
#include "pcharacter.h"
#include "comm.h"
#include "act.h"
#include "merc.h"


GROUP(clan);

CMDRUN( glist )
{
    ostringstream buf;
    HelpArticle::Pointer help;
    DLString argument = constArguments;
    
    if (!ch->getPC( ))
        return;

    if (argument.empty( )) {
        buf << "Все группы:" << endl << endl;;
        
        for (int gn = 0; gn < skillGroupManager->size( ); gn++) {
            SkillGroup *group = skillGroupManager->find( gn );

            if (group->isValid() && group->visible(ch))
                buf << fmt( 0, "    {g{hg%N1{x", group->getNameFor(ch).c_str())
                    << endl;
        }

        buf << endl
            << "Для просмотра навыков каждой группы используй '{yгруппаумений {D<{wгруппа{D>{w'."
            << endl;
    }
    else {
        SkillGroup *group = skillGroupManager->findUnstrict( argument );
        
        if (!group) {
            ch->pecho("Неправильно указана группа.");
            return;
        }

        if (!group->visible(ch)) {
            ch->pecho("Эта группа умений скрыта от тебя.");
            return;
        }
        
        group->show( ch->getPC( ), buf );
    }
    
    page_to_char( buf.str( ).c_str( ), ch );
}


