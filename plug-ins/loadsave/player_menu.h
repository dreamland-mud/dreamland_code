#ifndef PLAYER_MENU_H
#define PLAYER_MENU_H

#include "interpretlayer.h"

class PCharacter;
class DLString;
class Integer;
namespace Scripting {
    class RegisterList;
}

/**
 * Utilities to set/display/clear player menu: a list of choices with numeric input.
 */
namespace Player {
    void menuSet(PCharacter *pch, const Integer &choice, const DLString &action);

    const DLString & menuGet(PCharacter *pch, const Integer &choice);

    void menuClear(PCharacter *pch);

    bool menuAvailable(PCharacter *pch);

    void menuPrint(PCharacter *pch);
};

/**
 * An interpret layer handling command input consisting only of a single number,
 * to represent menu choices. 
 */
class MenuInterpretLayer : public InterpretLayer {
public:

    virtual void putInto();
    virtual bool process( InterpretArguments &iargs );
};

#endif
