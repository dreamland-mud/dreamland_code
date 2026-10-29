/* $Id$
 *
 * ruffina, 2004
 */
#include "helpcontainer.h"
#include "helpsave.h"

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

void GenericHelp::detach()
{
    // The container's list may hold the last reference.
    HelpArticle::Pointer keep(static_cast<HelpArticle *>(this));

    helpManager->unregistrate(keep);

    if (!container)
        return;

    for (auto i = container->begin(); i != container->end(); i++)
        if (i->getPointer() == this) {
            container->erase(i);
            break;
        }

    help_save_owner(this, container.getPointer());
    container.clear();
}

void GenericHelp::save() const
{
    if (container)
        help_save_owner(this, container.getPointer());
}