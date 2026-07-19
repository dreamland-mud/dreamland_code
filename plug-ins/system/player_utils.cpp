#include "player_utils.h"
#include "xmlattributestatistic.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "commonattributes.h"

/** 
 * Someone who lives their first life and haven't done enough successful quests
 * is considered a 'newbie'.
 */
bool Player::isNewbie(PCMemoryInterface *pcm)
{
    if (pcm->getRemorts().size() > 0)   
        return false;
    
    XMLAttributeStatistic::Pointer stats = pcm->getAttributes( ).findAttr<XMLAttributeStatistic>("questdata");
    if (stats && stats->getAllVictoriesCount() > 50)
        return false;
    
    return true;
}

lang_t Player::lang(Character *ch)
{
    if (ch->is_npc()) {
        if (ch->getNPC()->switchedFrom)
            return lang(ch->getNPC()->switchedFrom);
        else
            return LANG_DEFAULT;
    }

    auto langAttr = ch->getPC()->getAttributes().findAttr<XMLStringAttribute>("lang");
    if (!langAttr)
        return LANG_DEFAULT;

    // TODO create a lookup function
    if (langAttr->getValue() == "ua")
        return LANG_UA;
    if (langAttr->getValue() == "ru")
        return LANG_RU;
    if (langAttr->getValue() == "en")
        return LANG_EN;

    return LANG_DEFAULT;
}

lang_t Player::displayLang(Character *ch)
{
    if (!ch)
        return LANG_DEFAULT;

    if (ch->is_npc()) {
        if (ch->getNPC()->switchedFrom)
            return displayLang(ch->getNPC()->switchedFrom);
        else
            return LANG_DEFAULT;
    }

    // An explicit 'config lang' choice wins over everything.
    auto langAttr = ch->getPC()->getAttributes().findAttr<XMLStringAttribute>("lang");
    if (langAttr && !langAttr->getValue().empty())
        return lang(ch);

    // Never set a language: players who predate 'config lang' are seeded from
    // the retired 'rucommands' flag on load (PCharacterManager::load), so a
    // still-empty 'lang' here just means a brand-new session -- use the default.
    return LANG_DEFAULT;
}

DLString Player::title(PCMemoryInterface *pcm, lang_t lang)
{
    ostringstream out;
    const char *str = pcm->getTitle( ).c_str( );
    
    switch (str[0]) {
    case '.': case ',': case '!': case '?':
        break;
    default:
        out << " ";
        break;
    }

    for (; *str; str++) {
        if (*str == '%') {
            DLString cl;
            
            if (*++str == '\0')
                break;

            switch (*str) {
            default:
                out << *str;
                break;
            
            case 'c':
                cl = pcm->getClan()->getTitle(pcm);
                if (!cl.empty())
                    out << "{C[" << "{" << pcm->getClan()->getColor( ) << cl << "{C]{x";
                break;
                
            case 'C':
                cl = pcm->getClan()->getTitle(pcm);
                if (!cl.empty( )) {
                    cl.upperFirstCharacter( );
                    out << "{C[" << "{" << pcm->getClan()->getColor( ) << cl << "{C]{x";
                }
                break;
                
            case 'a':
                out << pcm->getProfession( )->getTitle(pcm, lang);
                break;
            }
        }
        else
            out << *str;
    }

    return out.str( );
}
