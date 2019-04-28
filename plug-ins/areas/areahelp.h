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
    void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );

    DLString keyword;
    int level;
    DLString labels;
};

class AreaHelp : public MarkupHelpArticle {
XML_OBJECT
public:
    typedef ::Pointer<AreaHelp> Pointer;

    virtual void getRawText( Character *, ostringstream & ) const;
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
