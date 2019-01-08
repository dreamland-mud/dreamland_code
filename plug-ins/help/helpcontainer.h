/* $Id$
 *
 * ruffina, 2004
 */
#ifndef HELPCONTAINER_H
#define HELPCONTAINER_H

#include "xmllist.h"
#include "xmltableelement.h"
#include "xmlpointer.h"
#include "markuphelparticle.h"

typedef XMLPointer<MarkupHelpArticle> MarkupHelpArticlePointer;

class HelpContainer : public XMLListContainer<MarkupHelpArticlePointer>,
                      public XMLTableElement
{
XML_OBJECT
public:

    virtual ~HelpContainer( );

    virtual void loaded( );
    virtual void unloaded( );

    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    
protected:    
    DLString name;
};


const DLString & HelpContainer::getName( ) const
{
    return name;
}

void HelpContainer::setName( const DLString &name )
{
    this->name = name;
}

#endif
