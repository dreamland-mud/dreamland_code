/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MARKUPHELPARTICLE_H
#define MARKUPHELPARTICLE_H

#include <sstream>
#include "helpmanager.h"

class MarkupHelpArticle : public HelpArticle {
public:
    typedef ::Pointer<MarkupHelpArticle> Pointer;

    virtual ~MarkupHelpArticle( );
    virtual DLString getText( Character * = NULL ) const;

protected:
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void applyFormatter( Character *, ostringstream &, ostringstream & ) const;
};

#endif
