/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PETSHOPSTORAGE_H__
#define __PETSHOPSTORAGE_H__

#include "roombehavior.h"
#include "trader.h"
#include "pet.h"


class PetShopStorage : public virtual RoomBehavior, public virtual Trader {
XML_OBJECT
friend class PetShopRoom;
public:
    typedef ::Pointer<PetShopStorage> Pointer;
    
    virtual bool isCommon( );
    virtual bool canEnter( Character * );

    virtual NPCharacter * getKeeper( ) const;

protected:
    virtual bool canServeClient( Character * );
    virtual Article::Pointer findArticle( Character *, DLString & );
    virtual void toStream( Character *, ostringstream & );

    virtual void msgListEmpty( Character * );
    virtual void msgListBefore( Character * );
    virtual void msgListAfter( Character * );
    virtual void msgListRequest( Character * );
    virtual void msgBuyRequest( Character * );
    virtual void msgArticleNotFound( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );

public:
    Pet::Pointer getPetBehavior( Character * ) const;
};


#endif
