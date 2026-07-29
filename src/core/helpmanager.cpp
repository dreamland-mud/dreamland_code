/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "helpmanager.h"
#include "character.h"
#include "multimessage.h"

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

DLString help_title_fmt(lang_t lang, const MultiMessage &pattern,
                        const DLString &a1, const DLString &a2)
{
    DLString result = pattern.getMessage(lang);
    result.replaces("%1$s", a1);
    if (!a2.empty())
        result.replaces("%2$s", a2);
    return result;
}

DLString HelpArticle::getTitle(const DLString &label) const
{
    const DLString &t = title.get(LANG_DEFAULT);

    if (!t.empty())
        return t;
    else
        return getAllKeywordsString();
}

DLString HelpArticle::getTitle(const DLString &label, lang_t lang) const
{
    // getForLang falls back to RU when the requested language is empty, so a
    // filled per-lang title upgrades the display and everything else stays RU.
    const DLString &t = title.getForLang(lang);

    if (!t.empty())
        return t;
    else
        return getTitle(label);
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
        keywordsAuto.insert(k.toUpper().quote());

    refreshKeywords();
}

void HelpArticle::addAutoKeyword(const StringSet &keywords)
{
    for (auto &k: keywords)
        keywordsAuto.insert(k.toUpper().quote());

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

/*
 * Help categories -- HELP_IA.md.
 *
 * The vocabulary is closed and an article carries at most one topical key in its
 * persistent labels. Most articles carry none: the labels the engine assigns at
 * load time (spell, social, race, area, <class>-skills ...) already say what the
 * article is, so an ordered fallback over those places about 1150 of the ~1340
 * player-visible articles without any hand labelling at all.
 *
 * Order is load-bearing in both lists. A spell also carries `skill` and its
 * class facet, so `spell` has to be tested before `skill` or every spell would
 * end up on the skills shelf.
 *
 * The same order is implemented in scripts/help_category.py (the specification)
 * and in dreamland_web/site.js/help-category.js (the website). All three must
 * agree; change them together.
 */
static const char * PLAYER_CATEGORIES[] = {
    "start", "char", "combat", "skills", "magic", "classes", "races", "gods",
    "items", "quests", "world", "society", "comm", "socials", NULL
};

static const char * NON_PLAYER_CATEGORIES[] = {
    "imm", "credits", "engine", "deprecated", NULL
};

static const struct {
    const char *facet;
    const char *category;
} CATEGORY_FALLBACK[] = {
    { "spell",        "magic"   },
    { "social",       "socials" },
    { "race",         "races"   },
    { "raceaptitude", "races"   },
    { "religion",     "gods"    },
    { "class",        "classes" },
    { "skillgroup",   "classes" },
    { "craft",        "items"   },
    { "craftskill",   "items"   },
    { "item",         "items"   },
    { "clanskill",    "skills"  },
    { "cardskill",    "skills"  },
    { "language",     "skills"  },
    { "skill",        "skills"  },
    { "area",         "world"   },
    { "clan",         "society" },
    { NULL,           NULL      }
};

const std::list<DLString> & HelpArticle::playerCategories()
{
    static std::list<DLString> categories;

    if (categories.empty())
        for (int i = 0; PLAYER_CATEGORIES[i]; i++)
            categories.push_back(PLAYER_CATEGORIES[i]);

    return categories;
}

bool HelpArticle::isPlayerCategory(const DLString &category)
{
    for (int i = 0; PLAYER_CATEGORIES[i]; i++)
        if (category == PLAYER_CATEGORIES[i])
            return true;

    return false;
}

DLString HelpArticle::getCategory() const
{
    for (int i = 0; PLAYER_CATEGORIES[i]; i++)
        if (labels.persistent.count(PLAYER_CATEGORIES[i]) > 0)
            return PLAYER_CATEGORIES[i];

    for (int i = 0; NON_PLAYER_CATEGORIES[i]; i++)
        if (labels.persistent.count(NON_PLAYER_CATEGORIES[i]) > 0)
            return NON_PLAYER_CATEGORIES[i];

    for (int i = 0; CATEGORY_FALLBACK[i].facet; i++)
        if (labels.all.count(CATEGORY_FALLBACK[i].facet) > 0)
            return CATEGORY_FALLBACK[i].category;

    // the generated per-class skill lists carry only a <class>-skills label
    for (auto &l: labels.all)
        if (l.size() > 7 && l.substr(l.size() - 7) == "-skills")
            return "classes";

    return DLString::emptyString;
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