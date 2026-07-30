/* $Id: cardskill.cpp,v 1.1.2.8.6.4 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2005
 */
#include "cardskill.h"
#include "mobiles.h"
#include "xmlattributecards.h"

#include "skillmanager.h"                                                       
#include "skillgroup.h"                                                       
#include "skill_utils.h"
#include "core/behavior/behavior_utils.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "room.h"
#include "npcharacter.h"
#include "merc.h"

#include "act.h"
#include "def.h"
#include "l10n.h"

GROUP(card_pack);
static GlobalBitvector cardGroups(skillGroupManager, group_card_pack);

CardSkill::CardSkill( )
{
}

GlobalBitvector & CardSkill::getGroups( ) 
{
    return cardGroups;
}

bool CardSkill::visible( CharacterMemoryInterface * ch ) const
{
    return isCard( ch );
}

bool CardSkill::available( Character * ch ) const
{
    return findCardLevel( ch ) >= cardLevel.getValue( );
}

bool CardSkill::usable( Character * ch, bool message = false ) const 
{
    return available( ch );
}

int CardSkill::getLevel( Character *ch ) const
{
    return 1;
}

int CardSkill::getLearned( Character *ch ) const
{
    if (!usable( ch, false ))
        return 0;

    return ch->getPC( )->getSkillData( getIndex( ) ).learned;
}

bool CardSkill::canPractice( PCharacter * ch, std::ostream & ) const
{
    return available( ch );
}

bool CardSkill::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose ) 
{
    if (mob && mob->behavior && mob->behavior.getDynamicPointer<CardSellerBehavior>( ))
        return true;

    if (verbose) {    
        if (mob)
            ch->pecho( _("%^C1 не разбирается в картах."), mob );
        else
            ch->pecho( _("Поищи кого-то, кто разбирается в картах.") );
    }

    return false;
}

/* The face a card skill starts appearing at. levelFaces carries Russian Flexer
 * pads only, so the sentence around it could not be translated without leaving a
 * Russian noun inside an English one; these are the same nine faces, in the
 * genitive the sentence needs. */
static const char * CARD_FACE_GENITIVE[3][9] = {
    // RU -- declined from levelFaces at runtime, this row is unused
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { "six", "seven", "eight", "nine", "ten", "jack", "queen", "king", "ace" },
    { "шістки", "сімки", "вісімки", "дев'ятки", "десятки",
      "валета", "дами", "короля", "туза" },
};

static DLString card_face_genitive(int level, lang_t lang)
{
    if (level < 0 || level > XMLAttributeCards::getMaxLevel())
        return DLString::emptyString;

    if (lang == LANG_EN || lang == LANG_UA)
        return CARD_FACE_GENITIVE[lang == LANG_EN ? 1 : 2][level];

    return russian_case( XMLAttributeCards::levelFaces[level].name, '2' );
}

void CardSkill::show( PCharacter *ch, std::ostream & buf ) const
{
    buf << fmt(ch, _("%1$s Колоды %2$s.{x"),
                   print_what(this, ch).c_str(),
                   print_names_for(this, ch).c_str())
        << endl;

    buf << print_group_for(this, ch);
    buf << printWaitAndMana(ch);
    
    buf << SKILL_INFO_PAD
        << fmt(ch, _("Появляется у карт, начиная с {C%1$s{x"),
                   card_face_genitive(cardLevel, Player::displayLang(ch)).c_str());

    if (visible( ch )) {
        int learned = getLearned( ch );
        if (learned > 0)
            buf << fmt(ch, _(", изучено на {%1$c%2$d%%{x"),
                           skill_learned_colour(this, ch), learned);

        if (!usable( ch ))
            buf << l(ch, " (сейчас тебе недоступно)");
    }
    
    buf << "." << endl; 
}

int CardSkill::getCategory() const
{
    return SKILL_CAT_CARDS;
}


/*---------------------------------------------------------------------------
 * 
 *---------------------------------------------------------------------------*/
int CardSkill::findCardLevel( CharacterMemoryInterface *mem ) 
{
    XMLAttributeCards::Pointer attr;
    
    if (!mem->getPCM())
        return -1;
    
    attr = mem->getPCM()->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );

    if (!attr)
        return -1;

    if (attr->isTrump( ))
        return XMLAttributeCards::getMaxLevel( );

    return attr->getLevel( );
}

bool CardSkill::isCard( CharacterMemoryInterface *mem )
{
    return findCardLevel( mem ) >= 0;
}
