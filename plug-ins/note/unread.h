/* $Id: unread.h,v 1.1.2.5.6.2 2007/09/11 00:34:18 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef UNREAD_H
#define UNREAD_H

#include "commandplugin.h"
#include "defaultcommand.h"
#include "descriptorstatelistener.h"

class PCharacter;

class Unread : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
        typedef ::Pointer<Unread> Pointer;
    
        Unread( );

        virtual void run( Character *, const DLString & );
        
        static void doNext( PCharacter * );
        static void doSpool( PCharacter *, bool );
        static void doUnfinished( PCharacter * );

private:
        static const DLString COMMAND_NAME;
                
};

class UnreadListener : public DescriptorStateListener {
public:
    typedef ::Pointer<UnreadListener> Pointer;
    
    virtual void run( int, int, Descriptor * );
    
};

#endif

