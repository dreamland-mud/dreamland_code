/* $Id: homerecall.h,v 1.1.2.3.6.1 2007/06/26 07:17:35 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef HOMERECALL_H
#define HOMERECALL_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlmap.h"
#include "xmlattribute.h"

#include "playerattributes.h"
#include "commandplugin.h"
#include "defaultcommand.h"

class PCharacter;

class HomeRecall : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
        typedef ::Pointer<HomeRecall> Pointer;
    
        HomeRecall( );

        virtual void run( Character *, const DLString & );
        
private:
        void doRecall( PCharacter *, const DLString & );
        void doSet( PCharacter *, DLString & );
        void doShow( PCharacter *, DLString & );
        void doRemove( PCharacter *, DLString & );
        void doList( PCharacter * );
        void doListMortal( PCharacter * );
        void doUsage( PCharacter * );

        static const DLString COMMAND_NAME;
                
};


class XMLAttributeHomeRecall : public RemortAttribute, public XMLVariableContainer
{
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeHomeRecall> Pointer;
        typedef XMLMapBase<XMLInteger> LabeledPoints;

        XMLAttributeHomeRecall( );
        virtual ~XMLAttributeHomeRecall( );

        int getLabeledPoint( const DLString & ) const;
        int getPoint( ) const;
        void setPoint( int, const DLString & );
        const LabeledPoints & getLabeled( ) const;

private:
        XML_VARIABLE XMLInteger point;
        XML_VARIABLE LabeledPoints labeled;
};

#endif

