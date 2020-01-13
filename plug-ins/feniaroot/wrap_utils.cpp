/* $Id$
 *
 * ruffina, 2004
 */
#include "wrap_utils.h"
#include "logstream.h"

#include "affect.h"
#include "pcharacter.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "room.h"
#include "skill.h" 
#include "skillmanager.h" 

#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "mobindexwrapper.h"
#include "structwrappers.h"
#include "affectwrapper.h"
#include "xmleditorinputhandler.h"

#include "subr.h"
#include "def.h"

Register wrap( ::Object * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( OBJ_INDEX_DATA * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( Character * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( MOB_INDEX_DATA * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( Room * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}

const Register & get_unique_arg( const RegisterList &args )
{
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    return args.front( );
}

int args2number( const RegisterList &args )
{
    return get_unique_arg( args ).toNumber( );
}

DLString args2string( const RegisterList &args )
{
    return get_unique_arg( args ).toString( );
}

DLString args2word( const RegisterList &args )
{
    StringSet ss;
    ss.fromString(get_unique_arg(args).toString());
    if (ss.size() > 1 || ss.size() < 1)
        throw Scripting::Exception("Expecting a single word or words in quotes");
    return *(ss.begin());
}

Character * args2character( const RegisterList &args )
{
    return wrapper_cast<CharacterWrapper>( get_unique_arg(args) )->getTarget( );
}

PCharacter * args2player( const RegisterList &args )
{
    Character *ch = args2character(args); 
    if (ch->is_npc())
        throw Scripting::CustomException("Mobile found when PC expected.");
    return ch->getPC();
}

Wearlocation * arg2wearloc( const Register &reg )
{
    DLString locName = reg.toString( );
    Wearlocation *loc = wearlocationManager->findExisting( locName );

    if (!loc)
        throw Scripting::CustomException( DLString("Invalid wearlocation name '") + locName + "'" );

    return loc;
}

::Object * arg2item( const Register &reg )
{
    return wrapper_cast<ObjectWrapper>( reg )->getTarget( );
}

Room * arg2room( const Register &reg )
{
    return wrapper_cast<RoomWrapper>( reg )->getTarget( );
}

Character * arg2character( const Register &reg )
{
    return wrapper_cast<CharacterWrapper>( reg )->getTarget( );
}

PCharacter * arg2player( const Register &reg )
{
    Character *ch = arg2character(reg);
    if (ch->is_npc())
        throw Scripting::CustomException("Mobile found when PC expected.");
    return ch->getPC();
}

void args2buf(const RegisterList &args, char *buf, size_t bufsize)
{
    strncpy(buf, args2string(args).c_str(), bufsize);
    buf[bufsize - 1] = 0;
}

static RegisterList::const_iterator argnum2iterator(const RegisterList &args, int num)
{
    if (args.size() < (unsigned int)num)
        throw Scripting::NotEnoughArgumentsException();
    
    RegisterList::const_iterator a;
    int i;
    for (a = args.begin(), i = 1; i <= num - 1 && a != args.end(); i++, a++) 
        ; 
    return a;
}

const Register & argnum(const RegisterList &args, int num)
{
    RegisterList::const_iterator a = argnum2iterator(args, num);
    return *a;
}

const Register & argnum2function(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    if (reg.type != Register::FUNCTION)
        throw Scripting::IllegalArgumentException();
    return reg;
}

RegisterList argnum2list(const RegisterList &args, int num)
{
    RegisterList::const_iterator a = argnum2iterator(args, num);
    RegisterList result;
    result.assign(a, args.end());
    return result;
}

Character *argnum2character(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return wrapper_cast<CharacterWrapper>(reg)->getTarget();
}

PCharacter *argnum2player(const RegisterList &args, int num)
{
    Character *ch = argnum2character(args, num);
    if (ch->is_npc())
        throw Scripting::CustomException("Mobile found when PC expected.");
    return ch->getPC();
}

int argnum2flag(const RegisterList &args, int num, const FlagTable &table)
{
    Register a = argnum(args, num);
    if (a.type == Register::STRING) {
        if (table.enumerated)
            return table.value( a.toString().c_str(), true );
        else
            return table.bitstring( a.toString().c_str(), true );
    }
    return a.toNumber();
}

int argnum2number(const RegisterList &args, int num)
{
    return argnum(args, num).toNumber();
}

DLString argnum2string(const RegisterList &args, int num)
{
    return argnum(args, num).toString();
}

Skill * args2skill( const RegisterList &args )
{
    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );

    return skillManager->findExisting( args.front( ).toString( ) );
}

Skill * argnum2skill(const RegisterList &args, int num)
{
    DLString name = argnum(args, num).toString();
    Skill *skill = skillManager->findExisting(name);
    if (!skill)
        throw Scripting::CustomException(name + ": skill name not found.");

    return skill;
}

