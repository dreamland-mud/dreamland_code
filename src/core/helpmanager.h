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
#include "xmlpersistent.h"

class Character;
struct area_file;

class HelpArticle : public DLString, public virtual DLObject, public virtual  XMLPolymorphVariable {
public:
    typedef ::Pointer<HelpArticle> Pointer;
    
    HelpArticle( );
    
    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );
    virtual DLString getText( Character * = NULL ) const;
    virtual bool visible( Character * ) const;
    /** Persist an XML file containing this article to disk. */
    virtual void save() const;

    /** Register new keyword for this article. */
    void addKeyword( const DLString & );
    void setText( const DLString & );
    /** Return a concatenation of all configured keywords. */
    const DLString & getKeyword( ) const;
    /** Return set of all configured keywords. */
    const StringSet & getKeywords() const;
    int getLevel( ) const;
    void setLevel( int );
    const DLString &getKeywordAttribute() const;
    void setKeywordAttribute(const DLString &);

    const StringSet &getLabels() const;
    void addLabel(const DLString &);
    void setLabelAttribute(const DLString &);
    void setID(int);
    int getID() const;
    virtual DLString getTitle(const DLString &label) const;

    struct area_file * areafile;

    static const DLString ATTRIBUTE_KEYWORD;
    static const DLString ATTRIBUTE_LEVEL;
    static const DLString ATTRIBUTE_REF;
    static const DLString ATTRIBUTE_REFBY;
    static const DLString ATTRIBUTE_LABELS;
   
protected:
    /** (Extra) keyword specified as an XML attribute for this help article. */
    DLString keyword;
    
    /** List of help articles this one refers to, specified as an XML attribute. */
    StringSet ref;

    /** List of help articles that refer to this one, specified as an XML attribute. */
    StringSet refby;

    /** Level from which this article is visible. */
    int level;

    /** Strings containing a concatenation of all keywords, with multi-word keywords
        inside single quotes. */
    DLString fullKeyword;

    /** A set of all configured keywords, single- and multi-word. Their current
        concatenation is kept inside fullKeyword field. */
    StringSet keywords;

    StringSet labels;
    DLString labelAttribute;

    int id;
};


typedef list<XMLPersistent<HelpArticle> > HelpArticles;
extern template class XMLStub<HelpArticle>;

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
