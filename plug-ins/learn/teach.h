/* $Id: teach.h,v 1.1.2.3.6.2 2008/02/23 13:41:24 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef TEACH_H
#define TEACH_H

#include "xmlvariablecontainer.h"
#include "xmlattribute.h"


class XMLAttributeTeacher : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT    
public:
        typedef ::Pointer<XMLAttributeTeacher> Pointer;

        XMLAttributeTeacher( );
        virtual ~XMLAttributeTeacher( );
};

#endif

