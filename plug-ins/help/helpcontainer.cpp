/* $Id$
 *
 * ruffina, 2004
 */
#include "helpcontainer.h"

TABLE_LOADER_IMPL(HelpLoader, "helps", "Help");


HelpContainer::~HelpContainer( )
{
}

void HelpContainer::loaded( )
{
    ::Pointer<HelpContainer> thiz(this);

    for (iterator i = begin( ); i != end( ); i++) {
        helpManager->registrate( *i );
        (*i)->setContainer(thiz);
    }
}

void HelpContainer::unloaded( )
{
    ::Pointer<HelpContainer> null;
    
    for (iterator i = begin( ); i != end( ); i++) {
        helpManager->unregistrate( *i );
        (*i)->setContainer(null);
    }
}

const DLString GenericHelp::TYPE = "Help";

const DLString & GenericHelp::getType( ) const
{
    return TYPE;
}

void GenericHelp::setContainer(::Pointer<HelpContainer> container)
{
    this->container = container;
}

void GenericHelp::save() const
{
    if (container)
        theHelpLoader->saveElement(container);
}