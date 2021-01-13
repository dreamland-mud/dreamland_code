/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MSGFORMATTER_H
#define MSGFORMATTER_H

#include "pointer.h"
#include "dlstring.h"

class Character;
class Skill;
namespace Grammar {
class Noun;
}

struct MsgFormatter {
    MsgFormatter(Character *to = NULL);
    virtual ~MsgFormatter();

protected:
    Character *to;
    const char *format;

    DLString run();
    DLString myupcase(const DLString &str, int upcase);
    DLString pad(bool left, int width, int limit, int upcase, const DLString &str);
    virtual void nextArg() = 0;
    virtual void shiftArg(int i) = 0;
    virtual char argChar() = 0;
    virtual int argInt() = 0;
    virtual float argFloat() = 0;
    virtual unsigned int argUInt() = 0;
    virtual DLString argStr() = 0;
    virtual const Skill * argSkill() = 0;
    virtual Pointer<Grammar::Noun> argNoun(int nounFlags = 0) = 0;
};

#endif
