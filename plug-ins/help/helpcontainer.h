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
#include "xmltableloaderplugin.h"

class HelpContainer;

/** A simple help article defined in one of the files in /help folder. */
class GenericHelp : public MarkupHelpArticle {
public:
    typedef XMLPointer<GenericHelp> Pointer;    
    
    virtual const DLString & getType( ) const;
    virtual void save() const;
    void setContainer(::Pointer<HelpContainer> container);

    static const DLString TYPE;
protected:
    ::Pointer<HelpContainer> container;
};

 /** A collection of articles from the /help folder. */
class HelpContainer : public XMLListContainer<GenericHelp::Pointer>,
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

TABLE_LOADER_DECL(HelpLoader);

#endif
