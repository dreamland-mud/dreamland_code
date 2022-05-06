/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SUMMONCREATURESPELL_H__
#define __SUMMONCREATURESPELL_H__

#include "xmlstring.h"
#include "xmllonglong.h"
#include "xmlinteger.h"
#include "xmlstringlist.h"
#include "spelltemplate.h"
#include "mobilebehavior.h"

class PCMemoryInterface;


class SummonedCreature : public virtual MobileBehavior {
XML_OBJECT
public:
    typedef ::Pointer<SummonedCreature> Pointer;
    
    virtual void conjure( );
    
    XML_VARIABLE XMLLongLong creatorID;
    XML_VARIABLE XMLString creatorName;
};

#endif
    
