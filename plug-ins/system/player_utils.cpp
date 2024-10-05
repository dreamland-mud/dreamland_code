#include "player_utils.h"
#include "xmlattributestatistic.h"
#include "pcharacter.h"
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

lang_t Player::lang(PCMemoryInterface *pcm)
{
    auto langAttr = pcm->getAttributes().findAttr<XMLStringAttribute>("lang");
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

DLString Player::title(PCMemoryInterface *pcm)
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
                out << pcm->getProfession( )->getTitle(pcm);
                break;
            }
        }
        else
            out << *str;
    }

    return out.str( );
}
