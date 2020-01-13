/* $Id: roomwrapper.h,v 1.1.4.8.6.2 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef _ROOMWRAPPER_H_
#define _ROOMWRAPPER_H_

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "xmlregister.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "pluginwrapperimpl.h"

class Room;

class RoomWrapper : public PluginWrapperImpl<RoomWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<RoomWrapper> Pointer;

    RoomWrapper( );

    virtual void setSelf( Scripting::Object * );
    virtual void extract( bool );
    void setTarget( Room *r );
    void checkTarget( ) const ;
    Room * getTarget( ) const;

private:
    Room *target;
};

#endif 
