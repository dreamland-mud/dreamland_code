/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "helpmanager.h"
#include "character.h"

template class XMLStub<HelpArticle>;

const DLString HelpArticle::ATTRIBUTE_LEVEL = "level";
const DLString HelpArticle::ATTRIBUTE_REF = "ref";
const DLString HelpArticle::ATTRIBUTE_REFBY = "refby";
const DLString HelpArticle::ATTRIBUTE_LABELS = "labels";
const DLString HelpArticle::ATTRIBUTE_ID = "id";

HelpArticle::HelpArticle( ) 
               : areafile( NULL ),
                 level( -1 ),
                 id(-1)
{
}

DLString HelpArticle::getText( Character * ) const
{
    return text.get(LANG_DEFAULT);
}

int HelpArticle::getLevel( ) const
{
    return level;
}

void HelpArticle::setLevel( int level )
{
    this->level = level;
}

void HelpArticle::setID(int id) 
{
    this->id = id;
}

int HelpArticle::getID() const
{
    return id;
}

DLString HelpArticle::getTitle(const DLString &label) const 
{
    const DLString &t = title.get(LANG_DEFAULT);

    if (!t.empty())
        return t;
    else
        return getAllKeywordsString();
}

void HelpArticle::save() const
{
    // Empty default impelemntation.
}

void HelpArticle::addAutoKeyword(const DLString &keyword)
{
    keywordsAuto.fromString(keyword.toUpper().quote());
    refreshKeywords();
}

void HelpArticle::addAutoKeyword(const std::list<DLString> &keywords)
{
    for (auto &k: keywords)
        keywordsAuto.insert(k.toUpper());

    refreshKeywords();
}

void HelpArticle::addAutoKeyword(const StringSet &keywords)
{
    for (auto &k: keywords)
        keywordsAuto.insert(k.toUpper());

    refreshKeywords();
}

void HelpArticle::refreshKeywords()
{
    keywordsAll.clear();
    keywordsAll.insert(keywordsAuto.begin(), keywordsAuto.end());
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        keywordsAll.fromString(keyword.get((lang_t)l));
    }

    keywordsAllString = keywordsAll.toString().toUpper();

    aka.clear();
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        aka.fromString(extra.get((lang_t)l));
    }
}

bool HelpArticle::visible( Character *ch ) const
{
    return ch->get_trust( ) >= level;
}

bool HelpArticle::toXML( XMLNode::Pointer &parent ) const
{
    XMLVariableContainer::toXML(parent);

    if (level >= -1)
        parent->insertAttribute( ATTRIBUTE_LEVEL, DLString( level ) );
    
    if (!ref.empty( ))
        parent->insertAttribute( ATTRIBUTE_REF, ref.toString( ) );

    if (!refby.empty( ))
        parent->insertAttribute( ATTRIBUTE_REFBY, refby.toString( ) );

    if (!labels.persistent.empty())
        parent->insertAttribute(ATTRIBUTE_LABELS, labels.persistent.toString());

    if (id > 0)
        parent->insertAttribute(ATTRIBUTE_ID, DLString(id));

    return true;
}

void HelpArticle::fromXML( const XMLNode::Pointer &parent ) 
{
    XMLVariableContainer::fromXML(parent);
    
    parent->getAttribute( ATTRIBUTE_LEVEL, level);
    parent->getAttribute(ATTRIBUTE_ID, id);
    ref.fromString( parent->getAttribute( ATTRIBUTE_REF ) );
    refby.fromString( parent->getAttribute( ATTRIBUTE_REFBY ) );
    labels.persistent.clear();
    labels.addPersistent(parent->getAttribute(ATTRIBUTE_LABELS));

    refreshKeywords();
}

/*-----------------------------------------------------------------------
 * HelpManager
 *-----------------------------------------------------------------------*/
HelpManager * helpManager = NULL;

HelpManager::HelpManager( )
{
    checkDuplicate( helpManager );
    helpManager = this;
}

HelpManager::~HelpManager( )
{
    helpManager = NULL;
}


void HelpManager::registrate( HelpArticle::Pointer art )
{
    articles.push_back( art );

    if (art->getID() > 0) {
        if (articlesById.count(art->getID()) > 0) {
            LogStream::sendError() << "Duplicate help ID " << art->getID() << " for "
                << art->getAllKeywordsString() << " and " << articlesById[art->getID()]->getAllKeywordsString() << endl;
            throw Exception("Duplicate help ID");
        }

        articlesById[art->getID()] = art;
    }
}

void HelpManager::unregistrate( HelpArticle::Pointer art )
{
    articles.remove( art );
    articlesById.erase(art->getID());
}

HelpArticle::Pointer HelpManager::getArticle(int id) const
{
    ArticlesById::const_iterator a = articlesById.find(id);
    if (a == articlesById.end())
        return HelpArticle::Pointer();
    else
        return a->second;
}

int HelpManager::getLastID() const
{
    HelpArticles::const_iterator a;
    int max_id = 0;

    for (a = getArticles( ).begin( ); a != getArticles( ).end( ); a++)
        if ((*a)->getID() > max_id)
            max_id = (*a)->getID();

    return max_id;
}