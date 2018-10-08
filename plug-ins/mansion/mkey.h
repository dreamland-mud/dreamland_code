/* $Id: mkey.h,v 1.1.2.4.6.3 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef MKEY_H
#define MKEY_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlvariablecontainer.h"

#include "xmlattribute.h"
#include "playerattributes.h"

#include "commandplugin.h"
#include "defaultcommand.h"

#include "trader.h"
#include "price.h"

class MKey : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
    typedef ::Pointer<MKey> Pointer;

    MKey( );

    virtual void run( Character*, const DLString& constArguments );
    
private:
    void doShow( Character *, DLString& );
    void doGrant( Character *, DLString& );
    void doRemove( Character *, DLString& );
    void usage( Character * );
    
    static const DLString COMMAND_NAME;
};

class MansionKeyMaker : public TraderBehavior {
XML_OBJECT
public:
    typedef ::Pointer<MansionKeyMaker> Pointer;

protected:
    virtual bool canServeClient( Character * );
    
    virtual void msgListEmpty( Character * );
    virtual void msgListBefore( Character * );
    virtual void msgListAfter( Character * );
    virtual void msgListRequest( Character * );

    virtual void msgBuyRequest( Character * );

    virtual void msgArticleNotFound( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );

    virtual void toStream( Character *, ostringstream & );
    virtual Article::Pointer findArticle( Character *, DLString & );

private:
    int findKeyVnum( PCharacter *, const DLString& );

    XML_VARIABLE XMLPointer<Price> price;
};

class MansionKeyArticle : public Article {
public:
    typedef ::Pointer<MansionKeyArticle> Pointer;

    virtual void purchase( Character *, NPCharacter *, const DLString &, int = 1 );
    virtual bool available( Character *, NPCharacter * ) const;
    virtual int getQuantity( ) const;

    inline void setVnum( int );
    inline void setPrice( Price::Pointer );
    
private:
    int vnum;
    Price::Pointer price;
};

inline void MansionKeyArticle::setVnum( int vnum )
{
    this->vnum = vnum;
}

inline void MansionKeyArticle::setPrice( Price::Pointer price )
{
    this->price = price;
}


class XMLAttributeMansionKey : public RemortAttribute, 
                               public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeMansionKey> Pointer;
    
    XML_VARIABLE XMLVectorBase<XMLInteger> keys;

};

#endif
