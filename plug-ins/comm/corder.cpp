/* $Id: corder.cpp,v 1.1.2.14.6.6 2009/01/01 20:07:46 rufina Exp $
 *
 * ruffina, 2004
 * logic based on 'do_order' from DreamLand 2.0
 */

#include "corder.h"
#include "commandinterpreter.h"

#include "pcharacter.h"
#include "room.h"
#include "interp.h"
#include "follow_utils.h"
#include "arg_utils.h"
#include "loadsave.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"
#include "morphology.h"

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

    // Lookup a matching command.
    interpretOrder( victim, iargs, argOrder );

    if (!iargs.pCommand) {
        ch->pecho("Похоже, такой команды не существует.");
        return;
    }

    // Check if command flags allow it to be ordered. Display verbose error message.
    int rc = iargs.pCommand->properOrder(victim);
    if (rc != RC_ORDER_OK) {
        switch (rc) {
            default:
            case RC_ORDER_ERROR:
                ch->pecho("Эту команду невозможно приказать.");
                break;
            case RC_ORDER_NOT_FIGHTING:
                ch->pecho("Эту команду можно приказать только в бою.");
                break;
            case RC_ORDER_NOT_PLAYER:
                ch->pecho("Эту команду можно приказать только игрокам.");
                break;
            case RC_ORDER_NOT_THIEF:
                ch->pecho("Эту команду можно приказать только ворам.");
                break;                
        }

        return;
    }

    // Check if victim can perform the command. Display verbose failure message to the master.
    rc = iargs.pCommand->dispatchOrder(iargs);
    if (rc != RC_DISPATCH_OK) {        
        switch (rc) {
            case RC_DISPATCH_AFK:
                victim->master->pecho("%1$^C1 не сможет выполнить эту команду, находясь в режиме AFK ('отошел').", victim);
                break;

            case RC_DISPATCH_PENALTY:
                victim->master->pecho("%1$^C1 наказан%1$Gо||а богами и не может выполнить эту команду.", victim);
                break;

            case RC_DISPATCH_GHOST:
                victim->master->pecho("%^C1 все еще призрак и не сможет выполнить эту команду.", victim);
                break;

            case RC_DISPATCH_SPELLOUT:
                victim->master->pecho("Эту команду необходимо указывать полностью.");
                break;

            case RC_DISPATCH_STUN:
                victim->master->pecho("%1$^C1 оглушен%1$Gо||а и ни на что не реагирует, подожди немного.", victim);
                break;

            case RC_DISPATCH_POSITION:
                victim->master->pecho(
                    "%1$#^C1 %3$sне может ходить и выполнять некоторые команды. Напиши {y{hc{lRприказать %2$s встать{lEorder %2$s stand{x.",
                    victim, 
                    Syntax::noun(victim->getNameP('1')).c_str(), 
                    victim->position == POS_SLEEPING ? "спит и " : (victim->position == POS_RESTING || victim->position == POS_SITTING) ? "сидит и " : "");
                break;

            case RC_DISPATCH_NOT_HERE:
            default:
                victim->master->pecho("Эта команда недоступна здесь и сейчас.");
                break;
        }

        return;
    }

    // Display exact message as typed by the master.
    victim->pecho("%^C1 приказывает тебе '%s', ты покорно исполняешь приказ.", ch, argOrder.c_str());

    // Run the command without further error feedback.
    iargs.pCommand->run( victim, iargs.cmdArgs );
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
