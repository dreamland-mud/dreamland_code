/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "helpmanager.h"
#include "character.h"

template class XMLStub<HelpArticle>;

const DLString HelpArticle::ATTRIBUTE_KEYWORD = "keyword";
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
    return *this;
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
    return getAllKeywordsString();
}


void HelpArticle::setText( const DLString &text )
{
    assign( text );
}

void HelpArticle::save() const
{
    // Empty default impelemntation.
}


const DLString &HelpArticle::getKeywordAttribute() const
{
    return keywordAttribute;
}

void HelpArticle::setKeywordAttribute(const DLString &keywordAttribute)
{
    this->keywordAttribute = keywordAttribute;
    refreshKeywords();
}

void HelpArticle::addAutoKeyword(const DLString &keyword)
{
    keywordsAuto.fromString(keyword.quote());
    refreshKeywords();
}

void HelpArticle::addAutoKeyword(const StringSet &keywords)
{
    keywordsAuto.insert(keywords.begin(), keywords.end());
    refreshKeywords();
}

void HelpArticle::refreshKeywords()
{
    keywordsAll.clear();
    keywordsAll.insert(keywordsAuto.begin(), keywordsAuto.end());
    keywordsAll.fromString(keywordAttribute);

    keywordsAllString = keywordsAll.toString().toUpper();
}

bool HelpArticle::visible( Character *ch ) const
{
    return ch->get_trust( ) >= level;
}

bool HelpArticle::toXML( XMLNode::Pointer &parent ) const
{
    XMLStringNoEmpty xmlString( *this );

    if (!xmlString.toXML( parent ))
        if (keywordAttribute.empty( ) && ref.empty( ) && refby.empty( ))
            return false;
    
    if (!keywordAttribute.empty( ))
        parent->insertAttribute( ATTRIBUTE_KEYWORD, keywordAttribute );

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

void HelpArticle::fromXML( const XMLNode::Pointer &parent ) throw (ExceptionBadType)
{
    XMLStringNoEmpty xmlString;
    
    xmlString.fromXML( parent );
    assign( xmlString );

    setKeywordAttribute(
        parent->getAttribute( ATTRIBUTE_KEYWORD ));

    parent->getAttribute( ATTRIBUTE_LEVEL, level);
    parent->getAttribute(ATTRIBUTE_ID, id);
    ref.fromString( parent->getAttribute( ATTRIBUTE_REF ) );
    refby.fromString( parent->getAttribute( ATTRIBUTE_REFBY ) );
    labels.persistent.clear();
    labels.addPersistent(parent->getAttribute(ATTRIBUTE_LABELS));
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
        if (articlesById.count(art->getID()) > 0)
            LogStream::sendError() << "Duplicate help ID " << art->getID() << " for "
                << art->getAllKeywordsString() << " and " << articlesById[art->getID()]->getAllKeywordsString() << endl;

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