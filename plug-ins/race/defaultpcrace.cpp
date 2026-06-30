/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultpcrace.h"
#include "profession.h"
#include "character.h"
#include "player_utils.h"
#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * DefaultPCRace
 *------------------------------------------------------------------*/
DefaultPCRace::DefaultPCRace( ) :
          classes( professionManager ),
          align( 0, &align_table )
{
}

GlobalArray & DefaultPCRace::getClasses( ) 
{
    return classes;
}
int DefaultPCRace::getPoints( ) const
{
    return points.getValue( );
}
int DefaultPCRace::getHpBonus( ) const
{
    return hpBonus.getValue( );
}
int DefaultPCRace::getManaBonus( ) const
{
    return manaBonus.getValue( );
}
int DefaultPCRace::getPracBonus( ) const
{
    return pracBonus.getValue( );
}
const Flags & DefaultPCRace::getAlign( ) const
{
    return align;
}
int DefaultPCRace::getMinAlign( ) const
{
    return minAlign.getValue( );
}
int DefaultPCRace::getMaxAlign( ) const
{
    return maxAlign.getValue( );
}


DLString DefaultPCRace::getWhoNameFor( Character *looker, Character *owner ) const
{
    // EN (and no viewer) get the latin who-tag; UA reuses the RU tag.
    if (!looker || Player::displayLang( looker ) == LANG_EN)
        return nameWho;

    if (!owner || owner->getSex( ) == SEX_MALE)
        return nameWhoRus;

    return nameWhoFemale;
}

DLString DefaultPCRace::getScoreNameFor( Character *looker, Character *owner ) const
{
    lang_t lang = looker ? Player::displayLang( looker ) : LANG_EN;

    if (lang == LANG_EN)
        return name;

    bool male = (!owner || owner->getSex( ) == SEX_MALE);

    // UA: gendered nominative from the UA field; fall back to RU when absent.
    if (lang == LANG_UA) {
        const DLString &ua = male ? getMaleNameUa( ) : getFemaleNameUa( );
        if (!ua.empty( ))
            return ua.ruscase( '1' );
    }

    if (!nameScore.empty( ))
        return nameScore;

    return (male ? nameMale : nameFemale).ruscase( '1' );
}


