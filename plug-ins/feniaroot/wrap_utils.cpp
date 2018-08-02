/* $Id$
 *
 * ruffina, 2004
 */
#include "wrap_utils.h"

#include "affect.h"
#include "pcharacter.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "room.h"

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

void args2buf(const RegisterList &args, char *buf, size_t bufsize)
{
    strncpy(buf, args2string(args).c_str(), bufsize);
    buf[bufsize - 1] = 0;
}

