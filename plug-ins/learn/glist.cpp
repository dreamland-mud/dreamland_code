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
#include "l10n.h"
#include "websocketrpc.h"

static DLString web_quoted(const DLString &name)
{
    if (name.find(' ') == DLString::npos)
        return name;
    return DLString("'") + name + "'";
}


GROUP(clan);

CMDRUN( glist )
{
    ostringstream buf;
    HelpArticle::Pointer help;
    DLString argument = constArguments;
    
    if (!ch->getPC( ))
        return;

    if (argument.empty( )) {
        WebList web(ch, "glist");

        buf << "Все группы:" << endl << endl;;
        
        for (int gn = 0; gn < skillGroupManager->size( ); gn++) {
            SkillGroup *group = skillGroupManager->find( gn );

            if (group->isValid() && group->visible(ch)) {
                buf << fmt( 0, "    {g{hg%N1{x", group->getNameFor(ch).c_str())
                    << endl;

                int row = web.row(fmt(0, "%N1", group->getNameFor(ch).c_str()), group->getName());
                web.action(row, "open", "glist " + web_quoted(group->getName()));
            }
        }

        web.send();

        buf << endl
            << "Для просмотра навыков каждой группы используй '{yгруппаумений {D<{wгруппа{D>{w'."
            << endl;
    }
    else {
        SkillGroup *group = skillGroupManager->findUnstrict( argument );
        
        if (!group) {
            ch->pecho(_("Неправильно указана группа."));
            return;
        }

        if (!group->visible(ch)) {
            ch->pecho(_("Эта группа умений скрыта от тебя."));
            return;
        }
        
        group->show( ch->getPC( ), buf );

        WebList web(ch, "group", group->getNameFor(ch));

        for (int sn = 0; web.active() && sn < skillManager->size( ); sn++) {
            Skill *skill = skillManager->find( sn );

            if (!skill->hasGroup(group->getIndex()))
                continue;

            int row = web.row(skill->getNameFor(ch), skill->getName());
            web.data(row, "usable", skill->usable(ch, false));
            if (skill->getSkillHelp() && skill->getSkillHelp()->getID() > 0)
                web.action(row, "help", "help " + DLString(skill->getSkillHelp()->getID()));
            else
                web.action(row, "help", "help " + skill->getName());
            web.action(row, "practice", "practice " + web_quoted(skill->getName()));
        }

        web.send();
    }
    
    page_to_char( buf.str( ).c_str( ), ch );
}


