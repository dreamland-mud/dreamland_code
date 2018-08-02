/* $Id: scribing.h,v 1.1.2.2.22.2 2008/02/23 13:41:31 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef SCRIBING_H
#define SCRIBING_H

#include "objectbehavior.h"
#include "objectbehaviorplugin.h"
#include "xmlmap.h"
#include "xmlinteger.h"

class SpellBook : public ObjectBehavior {
XML_OBJECT
public:
	typedef ::Pointer<SpellBook> Pointer;
	typedef XMLMapBase<XMLInteger> SpellList;
    
	SpellBook( );
	
	virtual bool examine( Character * );
	virtual DLString extraDescription( Character *ch, const DLString & );
        virtual bool hasTrigger( const DLString &  );
        void toString( ostringstream & buf );

	XML_VARIABLE SpellList spells;
};


#endif
