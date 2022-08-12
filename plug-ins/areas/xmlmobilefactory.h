/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSAVE_MOBILE_H__
#define __OLCSAVE_MOBILE_H__

#include "xmllist.h"
#include "xmlmap.h"
#include "xmlvariablecontainer.h"
#include "xmlglobalbitvector.h"
#include "xmlitemtype.h"

struct mob_index_data;

class XMLMobileFactory : public XMLVariableContainer {
XML_OBJECT
public:
    XMLMobileFactory( );

    void init(const mob_index_data *);
    void compat(mob_index_data *);
    mob_index_data *compat( );
    
    XML_VARIABLE XMLStringNoEmpty player_name, short_descr, long_descr, description;
    XML_VARIABLE XMLString race;
    XML_VARIABLE XMLFlagsDiff act, aff;
    XML_VARIABLE XMLIntegerNoEmpty alignment, group, level, hitroll;
    XML_VARIABLE XMLDice hit, mana, damage;
    XML_VARIABLE XMLEnumeration dam_type;
    XML_VARIABLE XMLArmor ac;
    XML_VARIABLE XMLFlagsDiff off, imm, res, vuln;
    XML_VARIABLE XMLEnumeration start_pos, default_pos, sex;
    XML_VARIABLE XMLIntegerNoEmpty wealth;
    XML_VARIABLE XMLFlagsDiff form, parts;
    XML_VARIABLE XMLEnumerationNoEmpty size;
    XML_VARIABLE XMLStringNoEmpty material;
    XML_VARIABLE XMLFlagsDiff detection;

    XML_VARIABLE XMLStringNoEmpty spec;
    XML_VARIABLE XMLGlobalBitvector practicer;
    XML_VARIABLE XMLGlobalBitvector religion;
    XML_VARIABLE XMLGlobalBitvector affects;
    XML_VARIABLE XMLStringNoEmpty gram_number;
    XML_VARIABLE XMLStringNoEmpty smell;

    XML_VARIABLE XMLStringNode behavior;
    XML_VARIABLE XMLMapBase<XMLString> properties;
};

#endif

