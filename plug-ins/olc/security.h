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
#include "webeditor.h"

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

class XMLAttributeOLC : public EventHandler<WebEditorSaveArguments>, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeOLC> Pointer;
    typedef XMLVectorBase<XMLVnumRange> RangeList;

    virtual bool handle( const WebEditorSaveArguments &args );
    void removeInterval( int, int );
    bool isOverlapping( int, int );

    /**
     * List of vnum ranges available to the builder and their security level.
     */
    XML_VARIABLE RangeList vnums;

    /**
     * Command (e.g. 'desc paste') that needs to be run after webedit is done.
     */
    XML_VARIABLE XMLString saveCommand;
};

#endif
