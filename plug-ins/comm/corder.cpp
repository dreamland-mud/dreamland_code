/* $Id: corder.cpp,v 1.1.2.14.6.6 2009/01/01 20:07:46 rufina Exp $
 *
 * ruffina, 2004
 * logic based on 'do_order' from DreamLand 2.0
 */

#include "corder.h"
#include "commandinterpreter.h"

#include "pcharacter.h"
#include "room.h"
#include "clanreference.h"
#include "skillreference.h"
#include "interp.h"

#include "follow_utils.h"
#include "arg_utils.h"
#include "loadsave.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"
#include "morphology.h"

CLAN(ruler);
GSN(manacles);

COMMAND(COrder, "order")
{
    Character *victim;
    InterpretArguments iargs;
    DLString argTarget, argOrder;
    DLString argument = constArguments;
    
    argTarget = argument.getOneArgument( );
    argOrder = argument;
    
    if (argTarget.empty( ) || argOrder.empty( )) {
        ch->pecho( "Приказать кому и что?" );
        return;
    }

    if (IS_CHARMED(ch)) {
        ch->pecho( "Ты можешь только принимать приказы, а не отдавать их." );
        return;
    }
    
    if (arg_is_all(argTarget)) {
        ch->pecho( "Ты не можешь отдать приказ всем сразу." );
        return;
    }
    
    victim = get_char_room(ch, argTarget, FFIND_FOR_ORDER|FFIND_INVISIBLE);

    if (!victim) {
        Character *follower = get_char_world(ch, argTarget, FFIND_FOLLOWER | FFIND_INVISIBLE);
        if (follower) {
            ch->pecho("Твой последователь должен быть рядом с тобой.");
            if (ch->getPC()->pet && follower->getNPC() && ch->getPC()->pet == follower->getNPC()) {
                interpret_raw(ch, "gtell", "где ты?");
            }
        } else {
            ch->pecho("Среди твоих последователей такого нет.");
        }
        return;
    }

    interpretOrder( victim, iargs, argOrder );

    if(victim->is_npc() && victim->master
    && iargs.pCommand && !iargs.pCommand->dispatchOrder( iargs )
    && victim->position < POS_FIGHTING)
    {
        DLString petName = Syntax::noun(victim->getNameP('1'));
        victim->master->pecho("%1$#^C1 %3$sне может ходить и выполнять некоторые команды. Напиши {y{hc{lRприказать %2$s встать{lEorder %2$s stand{x.",victim, petName.c_str(), victim->position == POS_SLEEPING ? "спит и " : (victim->position == POS_RESTING || victim->position == POS_SITTING) ? "сидит и " : "");
        return;
    }
    
    if (!iargs.pCommand) ch->pecho("Похоже, такой команды не существует.");
    
    else if(!iargs.pCommand->properOrder( victim )) {
        if (victim->isAffected( gsn_manacles ))
            oldact("$C1 говорит тебе '{GЯ не буду делать это.{x'", ch, 0, victim, TO_CHAR );
        else
            oldact("$C1 говорит тебе '{GЯ не понимаю, чего ты хочешь, хозя$gин|ин|йка.{x'", ch, 0, victim, TO_CHAR );
    }
    else {
        oldact("$c1 приказывает тебе '$t', ты покорно исполняешь приказ.", ch, iargs.pCommand->getName( ).c_str( ), victim, TO_VICT );
        
        if (iargs.pCommand->dispatchOrder( iargs ))
            iargs.pCommand->run( victim, iargs.cmdArgs );
    }


    ch->setWaitViolence( 1 );
    ch->pecho( "Ok.");
}


void COrder::interpretOrder( Character *och, InterpretArguments &iargs, const DLString &args )
{
    static int phases [] = { 
        CMDP_LOG_INPUT,
        CMDP_GRAB_WORD,
        CMDP_FIND,
        CMDP_LOG_CMD,
        0
    };

    iargs.ch = och;
    iargs.line = args;
    iargs.phases = phases;

    CommandInterpreter::getThis( )->run( iargs );
}
