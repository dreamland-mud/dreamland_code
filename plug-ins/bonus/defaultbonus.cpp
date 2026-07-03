#include "defaultbonus.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "lang.h"
#include "calendar_utils.h"
#include "act.h"
#include "merc.h"
#include "def.h"

DefaultBonus::DefaultBonus( )
                  : professions( professionManager )
{
}

void DefaultBonus::loaded( )
{
    bonusManager->registrate( Pointer( this ) );
}

void DefaultBonus::unloaded( )
{
    bonusManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultBonus::getName( ) const
{
    return Bonus::getName( );
}

void DefaultBonus::setName( const DLString &name ) 
{
    this->name = name;
}

const DLString &DefaultBonus::getRussianName( ) const
{
    return nameRus;
}

const DLString &DefaultBonus::getShortDescr( ) const
{
    return shortDescr;
}

char DefaultBonus::getColor() const
{
    if (color.size() < 1)
        return 'x';
    return color.at(0);
}

bool DefaultBonus::isValid( ) const
{
    return true;
}

bool DefaultBonus::available(PCharacter *ch) const
{
    if (!religious)
        return false;

    if (!professions.empty( ) && !professions.isSet( ch->getProfession( ) ))
        return false;
   
    return true; 
}

bool DefaultBonus::isActive(PCharacter *ch, const struct time_info_data &ti) const
{
    if (ch && activeForPlayer(ch, ti))
        return true;
    if (activeForAll(ti))
        return true;
    return false;
}

void DefaultBonus::reportAction(PCharacter *ch, ostringstream &buf) const
{
    lang_t lang = ch ? Player::lang(ch) : LANG_DEFAULT;

    if (activeForPlayer(ch, time_info)) {
        if (!msgActionReligion.getForLang(lang).empty())
            buf << fmt(0, msgActionReligion.getForLang(lang).c_str(), ch->getReligion()->getNameFor(ch).c_str()) << endl;
        return;
    }

    if (!msgActionGlobal.getForLang(lang).empty())
        buf << msgActionGlobal.getForLang(lang) << endl;
}

void DefaultBonus::reportTime(PCharacter *ch, ostringstream &buf) const
{
    lang_t lang = ch ? Player::lang(ch) : LANG_DEFAULT;

    if (activeForPlayer(ch, time_info))
        buf << fmt(0, msgTodayReligion.getForLang(lang).c_str(), ch->getReligion()->getNameFor(ch).c_str());
    else
        buf << msgTodayGlobal.getForLang(lang);
    buf << endl;
}


static bool check_value(const DLString &value, int i)
{
    if (value.isNumber()) {
        Integer boxed;
        return Integer::tryParse(boxed, value) && (i == boxed);
    }

    if (value == "even")
        return i % 2 == 0;
    if (value == "odd")
        return i % 2 != 0;
    if (value == "all")
        return true;

    return false;
}

bool DefaultBonus::activeForAll(const struct time_info_data &ti) const
{
    bool dayMatches = check_value(globalDay, ti.day);
    bool monthMatches = check_value(globalMonth, ti.month);
    return dayMatches && monthMatches;
}

bool DefaultBonus::activeForPlayer(PCharacter *ch, const struct time_info_data &ti) const
{
    if (!ch)
        return false;
        
    PCBonusData &data = ch->getBonuses().get(getIndex());
    long today = day_of_epoch(ti);
    return data.start <= today && today <= data.end;    
}

void ExperienceBonus::reportAction(PCharacter *ch, ostringstream &buf) const
{
    lang_t lang = ch ? Player::lang(ch) : LANG_DEFAULT;

    if (activeForPlayer(ch, time_info)) {
        buf << fmt(0, lmsg(lang,
                       "{c%^N1 grants you more experience.{x",
                       "{c%^N1 дарит тебе больше опыта.{x",
                       "{c%^N1 дарує тобі більше досвіду.{x"),
                   ch->getReligion()->getNameFor(ch).c_str());
    }
    else {
        const char *side = IS_GOOD(ch)
            ? lmsg(lang, "the forces of good", "силы добра", "сили добра")
            : IS_NEUTRAL(ch)
            ? lmsg(lang, "the neutral forces", "нейтральные силы", "нейтральні сили")
            : lmsg(lang, "the forces of evil", "силы зла", "сили зла");
        buf << fmt(0, lmsg(lang,
                       "{cToday %s favour you, granting you more experience.{x",
                       "{cСегодня %s благосклонны к тебе, даруя тебе больше опыта.{x",
                       "{cСьогодні %s прихильні до тебе, даруючи тобі більше досвіду.{x"),
                   side);
    }
    buf << endl;
}

