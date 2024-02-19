/* $Id: dlprintf.cpp,v 1.1.2.10.6.6 2010-08-24 20:26:47 rufina Exp $
 *
 * ruffina, 2004
 */

#include <ctype.h>

#include "register-impl.h"
#include "subr.h"

#include "noun.h"
#include "grammar_entities_impl.h"
#include "character.h"
#include "object.h"

#include "characterwrapper.h"
#include "objectwrapper.h"
#include "structwrappers.h"

#include "msgformatter.h"

#include "def.h"

using namespace Scripting;

#define MAXARGS 60

struct RegFormatter : public MsgFormatter {
    RegFormatter(Character *to) : MsgFormatter(to) {
    }
    DLString regfmt(const RegisterList &args) {
        RegisterList::const_iterator ipos = args.begin();
        
        if(ipos == args.end())
            throw Scripting::NotEnoughArgumentsException( );
        
        this->constFormat = ipos++->toString( );
        this->format = constFormat.c_str( );

        argc = args.size() - 1;
        if (argc >= MAXARGS)
            throw Scripting::TooManyArgumentsException();
        
        copy(ipos, args.end(), argv);
        argcnt = 0;
        
        return run();
    }

protected:
    virtual void nextArg() {
        if(argcnt < 0 || argcnt >= argc)
            throw Scripting::NotEnoughArgumentsException( );

        d = argv[argcnt++];
    }
    virtual void shiftArg(int i) {
        if(i < 0 || i > argc)
            throw Scripting::NotEnoughArgumentsException( );
            
        d = argv[i-1];
        argcnt = i;
    }
    virtual char argChar() {
        return d.toString().at(0);
    }
    virtual int argInt() {
        return d.toNumber();
    }
    virtual unsigned int argUInt() {
        return d.toNumber();
    }
    virtual float argFloat() {
        return d.toNumber();
    }
    virtual DLString argStr() {
        return d.toString();
    }
    virtual const Skill * argSkill() {
        SkillWrapper *skWrap = d.toHandler().getDynamicPointer<SkillWrapper>();
        if (!skWrap)
            throw Scripting::IllegalArgumentException();
        
        return skWrap->getTarget();
    }
    virtual Grammar::Noun::Pointer argNoun(int nounFlags) {
        CharacterWrapper *chWrap = d.toHandler().getDynamicPointer<CharacterWrapper>();
        if (chWrap != 0)
            return chWrap->getTarget()->toNoun(to, nounFlags);

        ObjectWrapper *objWrap = d.toHandler().getDynamicPointer<ObjectWrapper>(); 
        if (objWrap != 0)
            return objWrap->getTarget()->toNoun(to, nounFlags);
        
        throw Scripting::IllegalArgumentException();
    }

private:
    int argcnt;
    int argc;
    Register argv[MAXARGS], d;
    DLString constFormat;
};

DLString 
regfmt(Character *to, const RegisterList &args)
{
    // Workaround for situations when act/recho is invoked from 
    // obj progs on player load, while the player is still not in a room.
    if (to && to->in_room == NULL)
    return DLString::emptyString;

    RegFormatter formatter(to);
    return formatter.regfmt(args);
}

