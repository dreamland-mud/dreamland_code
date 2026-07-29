/* $Id$
 *
 * ruffina, 2004
 */
#ifndef HELPMANAGER_H
#define HELPMANAGER_H

#include "lang.h"
#include <list>
#include <map>

#include "oneallocate.h"
#include "xmlstringlist.h"
#include "xmlstring.h"
#include "xmlpersistent.h"
#include "xmlmultistring.h"
#include "xmlvariablecontainer.h"

class Character;
class MultiMessage;
struct area_file;

/** Resolve a help-title pattern in `lang` and fill its %N$s slots.
 *
 * Composed help titles ("Spell '<name>'", "Zone '<name>'", ...) are built in
 * a dozen different plug-ins, and not all of them link the output plug-in, so
 * fmtLang/act.h is not reachable everywhere. This does the one thing those
 * titles need -- a catalog lookup plus one or two substitutions -- from core,
 * where every help plug-in can already see it. */
DLString help_title_fmt(lang_t lang, const MultiMessage &pattern,
                        const DLString &a1,
                        const DLString &a2 = DLString::emptyString);

class HelpArticle : public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<HelpArticle> Pointer;
    
    HelpArticle( );
    
    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void fromXML( const XMLNode::Pointer& ) ;
    /** Return help article formatted for this char. */
    virtual DLString getText( Character * = NULL ) const;
    /** True if help article is visible (e.g. via level restrictions). */
    virtual bool visible( Character * ) const;
    /** Persist an XML file containing this article to disk. */
    virtual void save() const;

    /** Add new automatic keyword and refresh all keywords. */
    void addAutoKeyword(const DLString &keyword);
    /** Add new automatic keyword and refresh all keywords. */
    void addAutoKeyword(const std::list<DLString> &keywords);
    void addAutoKeyword(const StringSet &keywords);
    /** Return all automatic, non-editable keywords. */
    inline const StringSet & getAutoKeywords() const;

    /** Return all keywords as a single space-separated string, long words in single quotes. */
    inline const DLString &getAllKeywordsString() const;
    /** Return a set of all keywords. */
    inline const StringSet & getAllKeywords() const;

    int getLevel( ) const;
    void setLevel( int );

    int getID() const;
    void setID(int);

    /** Construct article title depending on implementation. */
    virtual DLString getTitle(const DLString &label) const;

    /** Same, but in the viewer's language: uses the per-lang title if present,
     *  otherwise falls back to the default (RU / keyword / subclass) title.
     *  Virtual so subclasses (e.g. BehaviorHelp) can render a fully per-viewer
     *  synthesized title without an explicit <title> field. */
    virtual DLString getTitle(const DLString &label, lang_t lang) const;

    /** Regenerate keywordsAll* fields. */
    void refreshKeywords();

    /** Browsable category this article belongs to, or an empty string if it
     *  belongs to none. See HELP_IA.md: at most one topical key is stored in
     *  'labels', and where there is none an ordered fallback over the labels the
     *  engine itself assigns decides. The website and the in-game browser both
     *  follow the same order, so a change here has to be mirrored in
     *  scripts/help_category.py and dreamland_web/site.js/help-category.js. */
    DLString getCategory() const;

    /** True if getCategory() names a category players are shown. Immortal docs,
     *  licences and engine-internal articles stay reachable by keyword and by
     *  direct link, but never appear in a browsable index. */
    static bool isPlayerCategory(const DLString &category);

    /** The player-facing categories, in display order. */
    static const std::list<DLString> & playerCategories();

    struct area_file * areafile;

    StringStorage labels;

    /** Values from 'extra' fields stored as a set for easy access. */
    StringSet aka;

    static const DLString ATTRIBUTE_KEYWORD;
    static const DLString ATTRIBUTE_LEVEL;
    static const DLString ATTRIBUTE_REF;
    static const DLString ATTRIBUTE_REFBY;
    static const DLString ATTRIBUTE_LABELS;
    static const DLString ATTRIBUTE_ID;
    
    /** Additional keywords, can be edited in OLC. */
    XML_VARIABLE XMLMultiString keyword;

    /** Overridden article title. */
    XML_VARIABLE XMLMultiString title;

    /** Hidden keywords, A.K.A., for odd spelling choices. Can be edited in OLC. */
    XML_VARIABLE XMLMultiString extra;
    
    XML_VARIABLE XMLMultiString text;
   
protected:
    /** A set of auto-generated keywords (coming from area name, skill name etc.) 
     *  Immutable after help is loaded.
     */
    StringSet keywordsAuto;
    
    /** A set of all keywords, automatic or extras, single- and multi-word. Their current
        concatenation is kept inside keywordsAllString field. */
    StringSet keywordsAll;

    /** Strings containing a concatenation of all keywords, with multi-word keywords
        inside single quotes. */
    DLString keywordsAllString;

    /** List of help articles this one refers to, specified as an XML attribute. */
    StringSet ref;

    /** List of help articles that refer to this one, specified as an XML attribute. */
    StringSet refby;

    /** Level from which this article is visible. */
    int level;

    /** Unique ID. */
    int id;

};

inline const DLString &HelpArticle::getAllKeywordsString() const
{
    return keywordsAllString;
}
inline const StringSet & HelpArticle::getAllKeywords() const
{
    return keywordsAll;
}
inline const StringSet & HelpArticle::getAutoKeywords() const
{
    return keywordsAuto;
}


typedef list<XMLPersistent<HelpArticle> > HelpArticles;
extern template class XMLStub<HelpArticle>;
typedef map<int, HelpArticle::Pointer> ArticlesById;

class HelpManager : public OneAllocate {
public:
    HelpManager( );    
    virtual ~HelpManager( );    

    void registrate( HelpArticle::Pointer );
    void unregistrate( HelpArticle::Pointer );
    inline const HelpArticles & getArticles( ) const;
    HelpArticle::Pointer getArticle(int id) const;
    int getLastID() const;

protected:
    HelpArticles articles; 
    ArticlesById articlesById;
};

extern HelpManager * helpManager;

inline const HelpArticles & HelpManager::getArticles( ) const
{
    return articles;
}

#endif
