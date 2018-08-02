/* $Id: xmllimits.h,v 1.1.2.4.24.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#ifndef XMLLIMITS_H
#define XMLLIMITS_H

#include "xmlclause.h"

typedef XMLListBase<XMLClause> XMLClausesList;

class XMLLimits : public XMLClausesList
{
public:
    typedef ::Pointer<XMLLimits> Pointer;

    XMLLimits( );
    virtual ~XMLLimits( );

    bool allow( DLObject * ) const;
    static const DLString TYPE;
};

#endif
