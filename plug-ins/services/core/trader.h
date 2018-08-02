/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __TRADER_H__
#define __TRADER_H__

#include <sstream>
#include "article.h"
#include "mobilebehavior.h"

class Character;

class Trader : public virtual DLObject {
public:
    
    virtual void doList( Character * );
    virtual void doBuy( Character *, const DLString & );
    virtual NPCharacter * getKeeper( ) const = 0;

protected:
    virtual bool canServeClient( Character * ) = 0;

    virtual void msgListEmpty( Character * ) = 0;
    virtual void msgListBefore( Character * ) = 0;
    virtual void msgListAfter( Character * ) = 0;
    virtual void msgListRequest( Character * ) = 0;
    
    virtual void msgBuyRequest( Character * ) = 0;

    virtual void msgArticleNotFound( Character * ) = 0;
    virtual void msgArticleTooFew( Character *, Article::Pointer ) = 0;

    virtual Article::Pointer findArticle( Character *, DLString & ) = 0;
    virtual void toStream( Character *, ostringstream & ) = 0;
};

class TraderBehavior : public virtual Trader, public virtual MobileBehavior {
public:

    virtual NPCharacter * getKeeper( ) const;
};

class Seller : public virtual Trader {
public:
    virtual void doSell( Character *, const DLString & );

protected:
    virtual void msgProposalNotFound( Character * ) = 0;
    virtual Article::Pointer findProposal( Character *, DLString & );
};

#endif
