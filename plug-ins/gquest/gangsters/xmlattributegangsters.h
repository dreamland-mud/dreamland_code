/* $Id: xmlattributegangsters.h,v 1.1.2.1.6.1 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEGANGSTERS_H
#define XMLATTRIBUTEGANGSTERS_H

#include "xmlinteger.h"                                                      
#include "xmlboolean.h"                                                      
#include "xmlattributeglobalquest.h"


class XMLAttributeGangsters : public XMLAttributeGlobalQuest {
XML_OBJECT
public: 
        typedef ::Pointer<XMLAttributeGangsters> Pointer;
    
	XMLAttributeGangsters( );

	int getKilled( ) const;
	void setKilled( int );
	void setJoined( );
	virtual bool isJoined( ) const;

private:	
	XML_VARIABLE XMLInteger killed;
	XML_VARIABLE XMLBoolean joined;
};


#endif

