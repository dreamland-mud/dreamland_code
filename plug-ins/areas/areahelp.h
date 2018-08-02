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

/* inheritance doesn't make sense in this case */
class AreaHelpArticle : public virtual MarkupHelpArticle {
public:
    typedef ::Pointer<AreaHelpArticle> Pointer;
    
    AreaHelpArticle( );
    AreaHelpArticle( const HelpArticle & );
    virtual ~AreaHelpArticle( );
};

class XMLAreaHelpArticle : public virtual AreaHelpArticle,
                           public virtual XMLHelpArticle
{
public:    
    XMLAreaHelpArticle( );
    XMLAreaHelpArticle( const HelpArticle & );
    virtual ~XMLAreaHelpArticle( );
    virtual bool toXML( XMLNode::Pointer& ) const;

    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;
};

inline const DLString & XMLAreaHelpArticle::getType( ) const
{
    return TYPE;
}


typedef XMLListBase<XMLAreaHelpArticle> XMLAreaHelpArticles;

#endif
