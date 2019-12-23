/* $Id: xmltableelement.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#include "xmltableelement.h"
#include "xmltableloader.h"

/*-------------------------------------------------------------------------
 * XMLTableElement
 *------------------------------------------------------------------------*/
void XMLTableElement::loaded( )
{
}

void XMLTableElement::unloaded( )
{
}


void XMLTableElement::save() const
{
    if (loader)
        loader->saveElement(this);
}

void XMLTableElement::setLoader(XMLTableLoader *loader)
{
    this->loader = loader;
}
