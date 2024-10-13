/* $Id$
 *
 * ruffina, 2009
 */
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "skillgroup.h"
#include "skill_utils.h"
#include "core/behavior/behavior_utils.h"
#include "pcharacter.h"
#include "pcrace.h"

#include "player_utils.h"
#include "math_utils.h"
#include "poliglot.h"

const DLString Language::CATEGORY = "Древние языки";
const int Language::SKILL_ADEPT  = 50;
const int Language::SKILL_SENSE  = 75;
const int Language::SKILL_NATIVE = 100;

GROUP(ancient_languages);
BONUS(learning);
static GlobalBitvector langGroups(skillGroupManager, group_ancient_languages);

GlobalBitvector & Language::getGroups( ) 
{
    return langGroups;
}

HelpArticlePointer Language::getSkillHelp( ) const
{
    return command->getHelp();
}

const DLString & Language::getRussianName( ) const
{
    return name.get(RU);
}

bool Language::matchesStrict( const DLString &str ) const 
{
    return name.matchesStrict(str);
}

bool Language::matchesUnstrict( const DLString &str ) const 
{
    return name.matchesUnstrict(str);
}

bool Language::matchesSubstring( const DLString &str ) const 
{
    return name.matchesSubstring(str);
}

const DLString& Language::getNameFor( Character *ch ) const
{
    return name.get(Player::lang(ch));
}

bool Language::visible( CharacterMemoryInterface * ch ) const
{
    if (ch->getPCM() && ch->getLevel( ) >= LEVEL_IMMORTAL)
        return true;
    
    if (!ch->getPCM())
        return false;

    const RaceLangInfo *race = getRaceInfo( ch );
    const ClassLangInfo *prof = getClassInfo( ch );
    
    return   (prof && prof->available( ))
          || (race && race->available( ));
}

bool Language::available( Character * ch ) const
{
    return ch->getRealLevel( ) >= getLevel( ch );
}

bool Language::usable( Character * ch, bool message = false ) const 
{
    if (ch->is_immortal( ) && ch->getRealLevel( ) >= LEVEL_IMMORTAL)
        return true;

    if (!available( ch ))
        return false;

    return true;
}

int Language::getLevel( Character *ch ) const
{
    if (ch->is_immortal( ) && ch->getRealLevel( ) >= LEVEL_IMMORTAL)
        return 1;

    if (!visible( ch ))
        return 999;

    const RaceLangInfo *race = getRaceInfo( ch->getPC( ) );
    const ClassLangInfo *prof = getClassInfo( ch->getPC( ) );

    return min(race->getLevel( ), prof->getLevel( ));
}

int Language::getLearned( Character *ch ) const
{
    int learned, maximum;
    
    if (ch->is_immortal( ) && ch->getRealLevel( ) > LEVEL_IMMORTAL)
        return SKILL_NATIVE;
        
    if (!usable( ch ))
        return 0;

    learned = ch->getPC( )->getSkillData( getIndex( ) ).learned.getValue( );
    maximum = getMaximum( ch );
    
    return min( learned, maximum );
}

bool Language::canPractice( PCharacter * ch, std::ostream & ) const
{
    return available( ch );
}

bool Language::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose )
{
    if (mob && mob->behavior && mob->behavior.getDynamicPointer<Poliglot>( ))
        return true;
   
    if (verbose) { 
        if (mob)
            ch->pecho( "%^C1 отнюдь не полиглот.", mob );
        else
            ch->pecho( "Поищи бродячего монаха." );
    }

    return false;
}

int Language::getAdept( PCharacter *ch ) const
{
    if (!visible( ch ))
        return 0;
    else
        return SKILL_ADEPT;
}

void Language::practice( PCharacter *ch ) const
{
    int &learned = ch->getSkillData( getIndex( ) ).learned;
    
    learned = max( learned, SKILL_ADEPT );
}

void Language::show( PCharacter *ch, std::ostream & buf ) const
{
    Races::const_iterator r;
    Classes::const_iterator c;
    WordList perfect, unperfect;
    WordList::const_iterator n;
    DLString userName;

    buf << "{" << SKILL_HEADER_BG << "Язык " << print_names_for(this, ch)
        << print_group_for(this, ch) << ".{x" << endl;

    for (r = races.begin( ); r != races.end( ); r++) {
        Race *race = raceManager->findExisting( r->first );
    
        if (!race || !race->isPC( ))
            continue;
            
        if (r->second.maximum == SKILL_NATIVE) 
            perfect.push_back( russian_case( race->getPC( )->getMltName( ), '1' ) );
        else 
            unperfect.push_back( russian_case( race->getPC( )->getMltName( ), '3' ) );
    }

    for (c = classes.begin( ); c != classes.end( ); c++) {
        Profession *prof = professionManager->findExisting( c->first );
        if (!prof)
            continue;

        if (r->second.maximum == SKILL_NATIVE)
            perfect.push_back( prof->getMltName( ).ruscase('1') );
        else 
            unperfect.push_back( prof->getMltName( ).ruscase('3') );
    }
    
    if (!perfect.empty( )) {
        buf << SKILL_INFO_PAD;
        for (n = perfect.begin( ); n != perfect.end( ); ) {
            userName = *n;

            if (n == perfect.begin( ))
                userName.upperFirstCharacter( );

            buf << "{W" << userName  << "{x";

            if (++n != perfect.end( ))
                buf << ", ";
        }
             
        buf << " могут овладеть этим языком в совершенстве." << endl;
    }

    if (!unperfect.empty( )) {
        buf << SKILL_INFO_PAD << "Неполные знания доступны ";
        
        for (n = unperfect.begin( ); n != unperfect.end( ); ) {
            buf << "{W" << *n << "{x";

            if (++n != unperfect.end( ))
                buf << ", ";
        }
        
        buf << "." << endl;
    }

    if (visible( ch )) {
        int learned = getLearned( ch );
        
        buf << SKILL_INFO_PAD << "Тебе язык доступен с уровня {C" << getLevel( ch ) << "{x";

        if (learned > 1)
            buf << ", изучен на {" << skill_learned_colour(this, ch) << learned << "%{x";
        else
            buf << ", пока не изучен";
        
        buf << "." << endl;
    }
}

int Language::getMaximum( Character *ch ) const
{
    if (ch->is_npc( ))
       return 0;
    
    int racemax = getRaceInfo( ch->getPC( ) )->getMaximum( );
    int profmax = getClassInfo( ch->getPC( ) )->getMaximum( );
    return max( racemax, profmax );
}

void Language::improve( Character *ch, bool, Character *victim, int, int ) const
{
    int learned = getLearned( ch );
    int maximum = getMaximum( ch );
    PCharacter *pch = ch->getPC( );

    if (learned < SKILL_ADEPT || learned >= maximum)
        return;

    if (maximum <= SKILL_ADEPT) 
        return;

    if (!chance( maximum / 2 ) && !bonus_learning->isActive(pch, time_info))
        return;
    
    if (number_percent( ) >= 4 * ch->getCurrStat( STAT_INT )) 
        return;
        
    PCSkillData &data = pch->getSkillData( getIndex( ) );

    data.learned++;
    ch->pecho( "Ты совершенствуешь свои познания в %^N6.", name.get(RU).c_str() );
}

AffectHandler::Pointer Language::getAffect( ) 
{
    return AffectHandler::Pointer( );
}
Spell::Pointer Language::getSpell( ) const 
{
    return Spell::Pointer( );
}
int Language::getBeats(Character *ch) const
{
    return ch ? percentage(beats, ch->mod_beats) : beats.getValue();
}

int Language::getMana( ) const
{
    return 0;
}

