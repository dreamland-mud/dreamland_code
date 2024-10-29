/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSAVE_XMLMISC_H__
#define __OLCSAVE_XMLMISC_H__

#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlnode.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"
#include "xmlglobalbitvector.h"
#include "xmlmultistring.h"

struct XMLArmor {
    XMLArmor( );

    void init(int, int, int, int);

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    int pierce, bash, slash, exotic;
};

struct XMLFlagsDiff {
    XMLFlagsDiff(const FlagTable *);

    void setRealBase(bitstring_t r, bitstring_t b);
    void setAddDel(bitstring_t a, bitstring_t d);

    bitstring_t get(bitstring_t base);

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    const FlagTable *table;
    bitstring_t add, del;
};


struct XMLDice {
    void set(int n, int t, int b);

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    int number, type, bonus;
};

struct XMLExtraDescription : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLString keyword;
    XML_VARIABLE XMLMultiString description;
};

struct XMLExtraDescr : public XMLStringNoEmpty {
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    DLString keyword;
};

struct XMLApply : public XMLIntegerNoEmpty {
    XMLApply( );

    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) ;

    bitstring_t location;
};

class Affect;

class XMLAffect : public XMLVariableContainer {
XML_OBJECT
public:
    void init(Affect *pAf);
    Affect * compat();

    XML_VARIABLE XMLFlagsWithTable bits;
    XML_VARIABLE XMLApply apply;
    XML_VARIABLE XMLGlobalBitvector global;
};

class XMLExitBase : public XMLVariableContainer {
XML_OBJECT
public:
    XMLExitBase( );

    XML_VARIABLE XMLMultiString keyword;
    XML_VARIABLE XMLMultiString description;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLIntegerNoDef<-1> key;
    XML_VARIABLE XMLIntegerNoDef<-1> target;
};

struct exit_data;

class XMLExitDir : public XMLExitBase {
XML_OBJECT
public:
    void init(const exit_data *);
    exit_data *compat( );

    XML_VARIABLE XMLMultiString short_descr;
};

struct extra_exit_data;

class XMLExtraExit : public XMLExitBase {
XML_OBJECT
public:
    XMLExtraExit( );

    void init(const extra_exit_data *);
    extra_exit_data *compat( );
    
    XML_VARIABLE XMLMultiString short_desc_from, short_desc_to, room_description;
    XML_VARIABLE XMLEnumeration max_size_pass;
    XML_VARIABLE XMLMultiString msgLeaveRoom, msgLeaveSelf;
    XML_VARIABLE XMLMultiString msgEntryRoom, msgEntrySelf;
};

#endif
