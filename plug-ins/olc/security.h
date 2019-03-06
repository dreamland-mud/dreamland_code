/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SECURITY_H__
#define __SECURITY_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlvector.h"

#include "xmlattribute.h"
#include "xmlattributeplugin.h"

class Character;
struct area_data;

class XMLVnumRange : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLVnumRange> Pointer;
    
    XMLVnumRange( );
    XMLVnumRange( int , int, int );

    XML_VARIABLE XMLInteger minVnum;
    XML_VARIABLE XMLInteger maxVnum;
    XML_VARIABLE XMLInteger security;
};

class XMLAttributeOLC : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeOLC> Pointer;
    typedef XMLVectorBase<XMLVnumRange> RangeList;

    void removeInterval( int, int );
    bool isOverlapping( int, int );

    XML_VARIABLE RangeList vnums;
};

class XMLAttributeOLCPlugin : public XMLAttributePlugin {
public:
        typedef ::Pointer<XMLAttributeOLCPlugin> Pointer;

        virtual void initialization( );
        virtual void destruction( );

        virtual const DLString& getName( ) const {
            return XMLAttributeOLC::MOC_TYPE;
        }
};

#endif
