/* $Id$
 *
 * ruffina, 2004
 */
#ifndef HELPMANAGER_H
#define HELPMANAGER_H

#include <list>

#include "oneallocate.h"
#include "xmlstringlist.h"
#include "xmlstring.h"
#include "xmlpolymorphvariable.h"

class Character;
struct area_file;

class HelpArticle : public DLString, public virtual DLObject {
public:
    typedef ::Pointer<HelpArticle> Pointer;
    
    HelpArticle( );
    
    virtual DLString getText( Character * = NULL ) const;
    virtual bool visible( Character * ) const;

    void addKeyword( const DLString & );
    void setText( const DLString & );
    const DLString & getKeyword( ) const;
    int getLevel( ) const;
    void setLevel( int );

    struct area_file * areafile;
protected:
    int level;
    DLString fullKeyword;
};

class XMLHelpArticle : public virtual HelpArticle, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<XMLHelpArticle> Pointer;

    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );

protected:
    static const DLString ATTRIBUTE_KEYWORD;
    static const DLString ATTRIBUTE_LEVEL;
    static const DLString ATTRIBUTE_REF;
    static const DLString ATTRIBUTE_REFBY;
    
    DLString keyword;
    StringSet ref, refby;
};

typedef list<HelpArticle::Pointer> HelpArticles;

class HelpManager : public OneAllocate {
public:
    HelpManager( );    
    virtual ~HelpManager( );    

    void registrate( HelpArticle::Pointer );
    void unregistrate( HelpArticle::Pointer );
    inline const HelpArticles & getArticles( ) const;

protected:
    HelpArticles articles; 
};

extern HelpManager * helpManager;

inline const HelpArticles & HelpManager::getArticles( ) const
{
    return articles;
}

#endif
