/* $Id: selfrate.h,v 1.1.2.3 2008/02/24 05:13:41 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef SELFRATE_H
#define SELFRATE_H

#include "playerattributes.h"
#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "xmlshort.h"
#include "xmlvariablecontainer.h"

class PCMemoryInterface;
class PCharacter;

bool rated_as_newbie( PCMemoryInterface* );
bool rated_as_expert( PCMemoryInterface* );
bool rated_as_guru( PCMemoryInterface* );

class XMLAttributeSelfRate : public EventHandler<WhoisArguments>,
                             public XMLVariableContainer {
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeSelfRate> Pointer;

        XMLAttributeSelfRate( );
        
        virtual bool handle( const WhoisArguments & );

        DLString getRateAlias( PCharacter *looker = NULL ) const;
        XML_VARIABLE XMLShort rate;
};

#endif

