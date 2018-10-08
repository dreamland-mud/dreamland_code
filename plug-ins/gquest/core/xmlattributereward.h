/* $Id: xmlattributereward.h,v 1.1.2.1.6.2 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEREWARD_H
#define XMLATTRIBUTEREWARD_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"

#include "descriptorstatelistener.h"
#include "xmlattribute.h"
#include "xmlattributeplugin.h"

class PCharacter;
class XMLReward;

class XMLReward : public XMLVariableContainer {
XML_OBJECT
public: 
        typedef ::Pointer<XMLReward> Pointer;
        
        XMLReward( );
	virtual ~XMLReward( );

	bool isEmpty( ) const;
	    
	XML_VARIABLE XMLInteger gold;
	XML_VARIABLE XMLInteger qpoints;
	XML_VARIABLE XMLInteger practice;
	XML_VARIABLE XMLInteger restring;
	XML_VARIABLE XMLInteger experience;
	XML_VARIABLE XMLString  reason;
	XML_VARIABLE XMLString  id;
};

class XMLAttributeReward : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT;
public:
	typedef ::Pointer<XMLAttributeReward> Pointer;

	void addReward( const XMLReward & );
	void reward( PCharacter * );

private:	
	XML_VARIABLE XMLVectorBase<XMLReward> rewards;
};


class XMLAttributeRewardListenerPlugin : public DescriptorStateListener {
public:
	typedef ::Pointer<XMLAttributeRewardListenerPlugin> Pointer;

	virtual void run( int, int, Descriptor * );	
};

#endif
