/* $Id: xmlattributeinvasion.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef XMLATTRIBUTEINVASION_H
#define XMLATTRIBUTEINVASION_H

#include "xmlattributeglobalquest.h"

class XMLAttributeInvasion : public XMLAttributeGlobalQuest {
XML_OBJECT
public: 
        typedef ::Pointer<XMLAttributeInvasion> Pointer;
        
        XMLAttributeInvasion( );

        int getKilled( ) const;
        void setKilled( int );
        bool isPunished( ) const;
        void punish( );

private:        
        XML_VARIABLE XMLInteger killed;
        XML_VARIABLE XMLBoolean punished;
};



#endif

