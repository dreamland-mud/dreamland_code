/* $Id: xmlattributeglobalquest.h,v 1.1.2.1 2005/09/10 21:13:00 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEGLOBALQUEST_H
#define XMLATTRIBUTEGLOBALQUEST_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmlboolean.h"

#include "xmlattributestatistic.h"
#include "xmlattributeplugin.h"

class XMLAttributeGlobalQuest : public XMLAttributeStatistic 
{
XML_OBJECT
public: 
        typedef ::Pointer<XMLAttributeGlobalQuest> Pointer;
	typedef XMLMapBase<XMLInteger> Victories;

	XMLAttributeGlobalQuest( );

	bool getNoExp( ) const;
	void setNoExp( bool );
	virtual bool isJoined( ) const;

protected:
	XML_VARIABLE XMLBoolean noexp;
};

#endif

