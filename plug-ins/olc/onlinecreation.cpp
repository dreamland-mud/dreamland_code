/* $Id$
 *
 * ruffina, 2004
 */

#include <sstream>

//#include "olc.h"
#include "plugininitializer.h"
#include "commandmanager.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "character.h"

#include "onlinecreation.h"

using std::endl;

OnlineCreation *OnlineCreation::ocList =  0;

OnlineCreation::OnlineCreation(struct cmd_info *ci)
                         : go(ci->go)
{
        next = ocList;
        ocList = this;
        
        name[EN] = ci->name;

        if (ci->rname && ci->rname[0])
            name[RU] = ci->rname;

        position.setValue(ci->position);
        level.setValue(0/*ci->level*/);
        log.setValue(ci->log);
        extra.setValue(ci->extra);
}

OnlineCreation::~OnlineCreation( )
{
}

void OnlineCreation::run( Character* ch, const DLString& args )
{
    if(!ch->desc)
        return;

    if(!available( ch )) {
        ch->pecho("Это не для тебя.");
        return;
    }
    
    (*go)(ch->getPC( ), (char*)args.c_str());
}

bool OnlineCreation::available( Character *ch ) const
{
    return (!ch->is_npc() && ch->getPC()->getSecurity() > 0);
}


/*-----------------------------------------------------------------------
 * OLCCommandLoader
 *-----------------------------------------------------------------------*/


CMDLOADER_DECL(olc)

CommandLoader * OnlineCreation::getLoader( ) const
{
    return CMDLOADER(olc)::getThis( );
}

