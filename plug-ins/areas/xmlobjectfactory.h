/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSAVE_OBJECT_H__
#define __OLCSAVE_OBJECT_H__

#include "xmllist.h"
#include "xmlmap.h"
#include "xmlvariablecontainer.h"
#include "xmlitemtype.h"
#include "xmlglobalbitvector.h"
#include "xmljsonvalue.h"
#include "xmlmultistring.h"

struct obj_index_data;

class XMLObjectFactory : public XMLVariableContainer {
XML_OBJECT
public:
    XMLObjectFactory( );

    void init(const obj_index_data *);
    obj_index_data * compat( );
    void compat(obj_index_data *);
    
    XML_VARIABLE XMLMultiString keyword, short_descr, description;
    XML_VARIABLE XMLStringNoEmpty name,  material; // name is compat
    XML_VARIABLE XMLFlagsNoEmpty extra_flags, wear_flags;
    XML_VARIABLE XMLItemType type;
    XML_VARIABLE XMLIntegerNoEmpty level, weight, cost;
    XML_VARIABLE XMLIntegerNoDef<100> condition;
    XML_VARIABLE XMLListBase<XMLAffect> affects;
    XML_VARIABLE XMLListBase<XMLExtraDescr> extraDescr; // compat
    XML_VARIABLE XMLListBase<XMLExtraDescription> extraDescriptions;
    XML_VARIABLE XMLStringNoEmpty gender;
    XML_VARIABLE XMLIntegerNoDef<-1> limit;
    XML_VARIABLE XMLMultiString sound, smell;
    XML_VARIABLE XMLStringNode behavior;
    XML_VARIABLE XMLGlobalBitvector behaviors;
    XML_VARIABLE XMLJsonValue props;
};

#endif
