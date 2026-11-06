#include <sstream>
#include <cstring>
#include "player_menu.h"
#include "pcharacter.h"
#include "register-impl.h"
#include "commonattributes.h"
#include "websocketrpc.h"
#include "commandinterpreter.h"
#include "descriptor.h"
#include "l10n.h"
#include "arg_utils.h"

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

    // The bracket label was a bare Russian literal, so every menu in the game --
    // quest answers included -- said "ВЫБОР" to English and Ukrainian players.
    // It is composed at runtime around the choice number, so it needs lmsg
    // rather than a catalog entry for the finished line.
    const char *label = lmsg(viewerLang(pch), "CHOICE ", "ВЫБОР ", "ВИБІР ");

    for (auto &m: menu) {
        const DLString &choice = m.first;
        const DLString &action = m.second;

        buf << "{D[" << label << choice << "]: {y{hc" << action << "{x" << endl;
    }

    pch->send_to(buf);

    if (is_websock(pch))
        pch->pecho(_("Напиши номер или нажми на вариант из списка."));
    else
        pch->pecho(_("Напиши номер из списка."));
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

// Menu choice from a typed word: "2", or dictated "2." / "два" / "two".
// 0 when the word is not a choice.
static int menu_choice_number(DLString word)
{
    static const char *keywords[] = { "one", "two", "three", "four", 0 };
    Integer choice;

    word.toLower();
    while (!word.empty() && strchr(".,!?;:", word.at(word.length() - 1)))
        word.erase(word.length() - 1);

    if (Integer::tryParse(choice, word))
        return choice.getValue();

    for (int i = 0; keywords[i]; i++)
        if (arg_is_strict(word, keywords[i]))
            return i + 1;

    return 0;
}

bool MenuInterpretLayer::process( InterpretArguments &iargs )
{
    Character *ch = iargs.d ? iargs.d->character : 0;

    if (!ch || ch->is_npc())
        return true;

    PCharacter *pch = ch->getPC();

    // A pasted option line "[ANSWER 2]: say hi" splits into "[" and the rest:
    // run what follows the label, menu or not.
    if (iargs.cmdName == "[") {
        DLString::size_type pos = iargs.cmdArgs.find("]:");
        if (pos != DLString::npos && pos < 32) {
            DLString rest = iargs.cmdArgs.substr(pos + 2);
            rest.stripWhiteSpace();
            if (!rest.empty()) {
                iargs.line = rest;
                iargs.splitLine();
            }
        }
        return true;
    }

    if (!Player::menuAvailable(pch))
        return true;

    // "answer 2", "відповідь 2": the label word in front of the number.
    DLString word = iargs.cmdName;
    if (!iargs.cmdArgs.empty()) {
        if (!arg_is_strict(iargs.cmdName.toLower(), "answer") || iargs.cmdArgs.find(' ') != DLString::npos)
            return true;
        word = iargs.cmdArgs;
    }

    int choice = menu_choice_number(word);
    if (choice <= 0)
        return true;

    // This choice is not from the latest menu.
    DLString menuAction = Player::menuGet(pch, Integer(choice));

    if (menuAction.empty()) {
        ch->pecho(_("{RТакого варианта не было в списке.{x\n"));            
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
