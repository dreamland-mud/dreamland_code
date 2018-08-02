/* $Id: xmlattributequestreward.h,v 1.1.4.1 2005/07/30 14:50:09 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef XMLATTRIBUTEQUESTREWARD_H
#define XMLATTRIBUTEQUESTREWARD_H

#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmlattribute.h"

class XMLAttributeQuestReward : public XMLAttribute, 
                                public XMLMapBase<XMLInteger>
{
public: 
        typedef ::Pointer<XMLAttributeQuestReward> Pointer;

	static const DLString TYPE;

	virtual const DLString & getType( ) const
	{
	    return TYPE;
	}

	int getCount( int ) const;
	void setCount( int, int );
};

#endif

