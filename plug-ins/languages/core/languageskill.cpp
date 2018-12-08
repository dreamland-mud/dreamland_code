/* $Id$
 *
 * ruffina, 2009
 */
#include "language.h"
#include "languagemanager.h"
#include "word.h"

#include "skillgroup.h"                                                       
#include "behavior_utils.h"
#include "pcharacter.h"
#include "pcrace.h"

#include "mercdb.h"

#include "poliglot.h"

const DLString Language::CATEGORY = "Древние языки";
const int Language::SKILL_ADEPT  = 50;
const int Language::SKILL_SENSE  = 75;
const int Language::SKILL_NATIVE = 100;

GROUP(ancient_languages);
BONUS(learning);

SkillGroupReference & Language::getGroup( ) 
{
    return group_ancient_languages;
}

bool Language::visible( Character * ch ) const
{
    if (!Command::available( ch ))
        return false;

    if (ch->is_immortal( ) && ch->getRealLevel( ) >= LEVEL_IMMORTAL)
        return true;
    
    if (ch->is_npc( ))
        return false;

    const RaceLangInfo *race = getRaceInfo( ch->getPC( ) );
    const ClassLangInfo *prof = getClassInfo( ch->getPC( ) );
    
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

int Language::getWeight( Character * ) const
{
    return 0;
}

bool Language::canForget( PCharacter * ) const
{
    return false;
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
            ch->println( "Поищи бродячего монаха." );
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

void Language::show( PCharacter *ch, std::ostream & buf ) 
{
    Races::iterator r;
    Classes::iterator c;
    WordList perfect, unperfect;
    WordList::iterator n;
    DLString userName;
    bool rus = ch->getConfig( )->ruskills;

    buf << "Язык '{W" << getName( ) << "{x'"
        << " '{W" << getRussianName( ) << "{x', "
        << "входит в группу '{hg{W" 
        << (rus ? getGroup( )->getRussianName( ) : getGroup( )->getName( )) 
        << "{x'"
        << endl;
    
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
        for (n = perfect.begin( ); n != perfect.end( ); ) {
            userName = *n;

            if (n == perfect.begin( ))
                userName.upperFirstCharacter( );

            buf << "{w" << userName  << "{x";

            if (++n != perfect.end( ))
                buf << ", ";
        }
             
        buf << " могут овладеть этим языком в совершенстве." << endl;
    }

    if (!unperfect.empty( )) {
        buf << "Неполные знания доступны ";
        
        for (n = unperfect.begin( ); n != unperfect.end( ); ) {
            buf << "{w" << *n << "{x";

            if (++n != unperfect.end( ))
                buf << ", ";
        }
        
        buf << "." << endl;
    }

    if (visible( ch )) {
        int learned = getLearned( ch );
        
        buf << "Тебе язык доступен с уровня {W" << getLevel( ch ) << "{x";

        if (learned > 0)
            buf << ", изучен на {W" << learned << "%{x";
        
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
    ch->pecho( "Ты совершенствуешь свои познания в %^N6.", nameRus.getValue( ).c_str( ) );
}

const DLString & Language::getName( ) const
{
    return Skill::getName( );
}
const DLString & Language::getHint( ) const
{
    return hint;
}

const Enumeration & Language::getPosition( ) const
{
    return Language::defaultPosition;
}

AffectHandler::Pointer Language::getAffect( ) 
{
    return AffectHandler::Pointer( );
}
Spell::Pointer Language::getSpell( ) const 
{
    return Spell::Pointer( );
}
int Language::getBeats( ) const
{
    return beats.getValue( );
}
int Language::getMana( ) const
{
    return 0;
}

