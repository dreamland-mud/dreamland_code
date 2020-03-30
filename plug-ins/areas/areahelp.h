/* $Id$
 *
 * ruffina, 2004
 */
#ifndef AREAHELP_H
#define AREAHELP_H

#include "oneallocate.h"
#include "plugin.h"
#include "xmllist.h"
#include "xmlpersistent.h"
#include "markuphelparticle.h"

class XMLAreaHelp : public XMLString {
XML_OBJECT
public:
    typedef ::Pointer<XMLAreaHelp> Pointer;

    XMLAreaHelp();
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& );

    DLString keywordAttribute;
    int level;
    DLString labels;
    int id;
    DLString titleAttribute;
};

class AreaHelp : public MarkupHelpArticle {
XML_OBJECT
public:
    typedef ::Pointer<AreaHelp> Pointer;

    virtual DLString getTitle(const DLString &label) const;
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void save() const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;
    
    bool selfHelp;
    bool persistent;
};

inline const DLString & AreaHelp::getType( ) const
{
    return TYPE;
}

#endif
