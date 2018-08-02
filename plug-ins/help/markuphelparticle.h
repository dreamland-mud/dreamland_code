/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MARKUPHELPARTICLE_H
#define MARKUPHELPARTICLE_H

#include <sstream>
#include "helpmanager.h"

class MarkupHelpArticle : public virtual HelpArticle {
public:
    typedef ::Pointer<MarkupHelpArticle> Pointer;

    virtual DLString getText( Character * = NULL ) const;

protected:
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void applyFormatter( Character *, ostringstream &, ostringstream & ) const;
};

class XMLMarkupHelpArticle : public virtual MarkupHelpArticle,
                             public virtual XMLHelpArticle
{
public:
    typedef ::Pointer<XMLMarkupHelpArticle> Pointer;

    virtual ~XMLMarkupHelpArticle( );
    
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;
};

inline const DLString & XMLMarkupHelpArticle::getType( ) const
{
    return TYPE;
}

#endif
