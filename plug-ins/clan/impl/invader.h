/* $Id: invader.h,v 1.1.6.1.10.3 2009/08/10 01:06:51 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef INVADER_H 
#define INVADER_H 

#include "clanmobiles.h"
#include "commandplugin.h"
#include "defaultcommand.h"

class ClanGuardInvader: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardInvader> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual int getCast( Character * );
};

class CDarkLeague : public CommandPlugin, public DefaultCommand {
public:
    typedef ::Pointer<CDarkLeague> Pointer;

    CDarkLeague( );
    virtual void run( Character *, const DLString & );
    virtual bool visible( Character * ) const;

private:
    void doUsage( PCharacter * );

    static const DLString COMMAND_NAME;
};


#endif
