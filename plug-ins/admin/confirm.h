/* $Id: confirm.h,v 1.1.2.7.6.1 2007/06/26 07:06:32 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef CONFIRM_H
#define CONFIRM_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmldate.h"
#include "xmlboolean.h"

#include "xmlattributeplugin.h"
#include "xmlattribute.h"
#include "commandplugin.h"
#include "defaultcommand.h"
#include "descriptorstatelistener.h"

class Character;

class Confirm : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
        typedef ::Pointer<Confirm> Pointer;
    
        Confirm( );

        virtual void run( Character*, const DLString& constArguments );
        
private:
        void doRequest( Character * );
        void doAccept( Character *, DLString& );
        void doReject( Character *, DLString& );
        void doDelete( Character *, DLString& );
        void doList( Character * );
        void doShow( Character *, DLString& );
        void doUnread( Character * );
        void usage( Character * );
        
        static const DLString COMMAND_NAME;
};


class XMLAttributeConfirm : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeConfirm> Pointer;

        void run( Character * );
        void update( Character * );

        XML_VARIABLE XMLString description;
        XML_VARIABLE XMLDate   date;
        XML_VARIABLE XMLString responsible;
        XML_VARIABLE XMLString reason;
        XML_VARIABLE XMLBoolean accepted;

};

class Descriptor;

class XMLAttributeConfirmListenerPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<XMLAttributeConfirmListenerPlugin> Pointer;
        
        virtual void run( int, int, Descriptor * );
};

#endif

