#include <sstream>
#include "player_menu.h"
#include "pcharacter.h"
#include "register-impl.h"
#include "commonattributes.h"
#include "websocketrpc.h"
#include "commandinterpreter.h"
#include "descriptor.h"

using namespace std;

static const DLString MENU_ATTR = "menu";

void Player::menuSet(PCharacter* pch, const Integer& choice, const DLString& action)
{
    set_map_attribute_value(pch, MENU_ATTR, choice.toString(), action);
}

void Player::menuClear(PCharacter* pch)
{
    pch->getAttributes().eraseAttribute(MENU_ATTR);
}

bool Player::menuAvailable(PCharacter* pch)
{
    return pch->getAttributes().isAvailable(MENU_ATTR);
}

void Player::menuPrint(PCharacter* pch)
{
    ostringstream buf;
    auto &menu = get_map_attribute(pch, MENU_ATTR);

    if (menu.empty())
        return;

    for (auto &m: menu) {
        const DLString &choice = m.first;
        const DLString &action = m.second;

        buf << "{D[ВЫБОР " << choice << "]: {y{hc" << action << "{x" << endl;
    }

    pch->send_to(buf);

    if (is_websock(pch))
        pch->pecho("Напиши номер или нажми на вариант из списка.");
    else
        pch->pecho("Напиши номер из списка.");
}

const DLString& Player::menuGet(PCharacter* pch, const Integer& choice)
{
    return get_map_attribute_value(pch, MENU_ATTR, choice.toString());
}


/**
 * An interpret layer handling command input consisting only of a single number,
 * to represent menu choices. Substitute number input by player with a corresponding
 * command from the "menu" attribute.
 */
void MenuInterpretLayer::putInto( )
{
    // Put this before any other layers such as socials, common commands, 
    // but after command line has been split and sanitized.
    // NOTE: aliases can still mess up with menu selection
    interp->put( this, CMDP_FIND, CMD_PRIO_FIRST + 1 );
}

bool MenuInterpretLayer::process( InterpretArguments &iargs )
{
    Character *ch = iargs.d ? iargs.d->character : 0;
    Integer choice;

    if (!ch || ch->is_npc())
        return true;

    PCharacter *pch = ch->getPC();

    // Input is something other than ^[0-9]+$
    if (!iargs.cmdArgs.empty() || !Integer::tryParse(choice, iargs.cmdName)) {
        return true;
    }

    if (!Player::menuAvailable(pch))
        return true;

    // This choice is not from the latest menu.
    DLString menuAction = Player::menuGet(pch, choice);

    if (menuAction.empty()) {
        ch->pecho("{RТакого варианта не было в списке.{x\n");            
        Player::menuPrint(pch);
        return false;
    }

    // Substitute input command with remembered menu option, re-parse arguments.
    iargs.line = menuAction;
    iargs.splitLine();

    // Wipe the menu after first successful choice
    Player::menuClear(pch);
    return true;
}

