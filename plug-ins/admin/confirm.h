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
#include "descriptorstatelistener.h"

class Character;

class Confirm : public CommandPlugin {
XML_OBJECT
public:
        typedef ::Pointer<Confirm> Pointer;
    
        Confirm( );

        virtual void run( Character*, const DLString& constArguments );
        
        static void doRequest( Character * );
        static void doAccept( Character *, DLString& );
        static void doReject( Character *, DLString& );
        static void doDelete( Character *, DLString& );
        static void doList( Character *, bool newOnly );
        static void doUnread( Character * );
        static void doShow( Character *, DLString& );
        static void usage( Character * );
        
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

