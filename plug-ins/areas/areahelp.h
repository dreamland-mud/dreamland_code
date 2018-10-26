/* $Id$
 *
 * ruffina, 2004
 */
#ifndef AREAHELP_H
#define AREAHELP_H

#include "oneallocate.h"
#include "plugin.h"
#include "xmllist.h"
#include "markuphelparticle.h"


class AreaHelp : public virtual XMLHelpArticle,
                 public virtual MarkupHelpArticle {
XML_OBJECT
public:
    typedef ::Pointer<AreaHelp> Pointer;

    virtual void getRawText( Character *, ostringstream & ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;
    
    bool selfHelp;
};

inline const DLString & AreaHelp::getType( ) const
{
    return TYPE;
}
#endif
