/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANORG_H__
#define __CLANORG_H__

#include "xmlrussianstring.h"
#include "xmlmap.h"
#include "xmlstring.h"

class PCMemoryInterface;
class PCharacter;

class ClanOrder : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ClanOrder> Pointer;
    
    virtual bool canInduct( PCMemoryInterface * ) const;
    virtual const DLString &getTitle( PCMemoryInterface * ) const;

    XML_VARIABLE XMLString          name;
    XML_VARIABLE XMLStringNoEmpty   shortDescr;
};

typedef XMLPointer<ClanOrder> XMLClanOrder;

class ClanOrgs : public XMLMapContainer<XMLClanOrder> {
XML_OBJECT
public:
    ClanOrder::Pointer findOrder( PCMemoryInterface * ) const;
    ClanOrder::Pointer findOrder( const DLString & ) const;

    void doList( PCharacter * ) const;
    void doMembers( PCharacter * ) const;
    void doSelfInduct( PCharacter *, DLString & ) const;
    void doInduct( PCharacter *, DLString & ) const;
    void doSelfRemove( PCharacter * ) const;
    void doRemove( PCharacter *, DLString & ) const;

    static const DLString &getAttr( PCMemoryInterface * );
    static void setAttr( PCMemoryInterface *, const DLString & );
    static void delAttr( PCMemoryInterface * );
    static bool hasAttr( PCMemoryInterface * );
    
    XML_VARIABLE XMLRussianString name;
    
    static const DLString ATTR_NAME;
};

#endif
