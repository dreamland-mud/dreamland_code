/* $Id: xmlattributeinduct.h,v 1.1.6.4 2005/11/21 19:31:05 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef XMLATTRIBUTEINDUCT_H
#define XMLATTRIBUTEINDUCT_H

#include "xmlattribute.h"
#include "xmlvariablecontainer.h"
#include "xmlvector.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "xmlinteger.h"

#include "descriptorstatelistener.h"
#include "clanreference.h"

class PCharacter;

class XMLInductEntry : public XMLVariableContainer {
XML_OBJECT    
public:    
    typedef ::Pointer<XMLInductEntry> Pointer;
    
    XML_VARIABLE XMLString message;
    XML_VARIABLE XMLBoolean update;
    XML_VARIABLE XMLClanReference prevClan;
};

class XMLAttributeInduct : public XMLAttribute, 
			   public XMLVectorBase<XMLInductEntry>
{
public:
	typedef ::Pointer<XMLAttributeInduct> Pointer;

        virtual const DLString &getType( ) const 
	{
            return TYPE;
        }

	static const DLString TYPE;                                             
	
	void run( PCharacter * );
	void addEntry( DLString );
};

class XMLAttributeInductListenerPlugin : public DescriptorStateListener {
public:
	typedef ::Pointer<XMLAttributeInductListenerPlugin> Pointer;


	virtual void run( int, int, Descriptor * );
};

#endif

