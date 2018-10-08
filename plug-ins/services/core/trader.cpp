/* $Id$
 *
 * ruffina, 2004
 */
#include "trader.h"

#include "pcharacter.h"
#include "npcharacter.h"

/*----------------------------------------------------------------------
 * Trader
 *---------------------------------------------------------------------*/
void Trader::doList( Character *client )
{
    ostringstream buf;
    
    msgListRequest( client );

    if (!canServeClient( client ))
        return;
    
    toStream( client, buf );

    if (buf.str( ).empty( ))
        msgListEmpty( client );
    else {
        msgListBefore( client );
        client->send_to( buf );
        msgListAfter( client );
    }
}

void Trader::doBuy( Character *client, const DLString &constArguments )
{
    int quantity;
    Article::Pointer article;
    DLString arguments, arg;

    msgBuyRequest( client );

    if (!canServeClient( client ))
        return;

    arguments = constArguments;
    arg = arguments.getOneArgument( );
    quantity = arg.getMultArgument( );
    article = findArticle( client, arg );

    if (!article) {
        msgArticleNotFound( client );
        return;
    }
    
    if (!article->available( client, getKeeper( ) )) 
        return;

    if (quantity > article->getQuantity( )) {
        msgArticleTooFew( client, article );
        return;
    }
    
    article->purchase( client, getKeeper( ), arguments, quantity );
}


/*----------------------------------------------------------------------
 * TraderBehavior
 *---------------------------------------------------------------------*/
NPCharacter * TraderBehavior::getKeeper( ) const
{
    return ch;
}

/*----------------------------------------------------------------------
 * Seller 
 *---------------------------------------------------------------------*/
void Seller::doSell( Character *client, const DLString &constArguments )
{
    Article::Pointer article;
    DLString arguments, arg;

    if (!canServeClient( client ))
        return;
    
    arguments = constArguments;
    arg = arguments.getOneArgument( );
    article = findProposal( client, arg );

    if (!article) {
        msgProposalNotFound( client );
        return;
    }

    article->sell( client, getKeeper( ) );
}

Article::Pointer Seller::findProposal( Character *client, DLString &arg )
{
    Article::Pointer null, article = findArticle( client, arg );

    if (!article)
        return null;

    if (!article->sellable( client ))
        return null;

    return article;
}

