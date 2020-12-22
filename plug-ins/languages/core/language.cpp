/* $Id: language.cpp,v 1.1.2.4 2010-08-24 20:33:31 rufina Exp $
 *
 * ruffina, 2005
 */
#include "logstream.h"
#include "language.h"
#include "languagemanager.h"
#include "word.h"
#include "wordeffect.h"
#include "xmlattributelanguage.h"

#include "skillmanager.h"                                                       
#include "commandmanager.h"

#include "pcharacter.h"
#include "pcrace.h"

#include "merc.h"
#include "mercdb.h"
#include "def.h"

/*--------------------------------------------------------------------
 * LanguageHelp
 *-------------------------------------------------------------------*/
const DLString LanguageHelp::TYPE = "LanguageHelp";

void LanguageHelp::save() const
{
   if (command) {
        const Language *lang = command.getDynamicPointer<Language>();
        if (lang)
            languageManager->saveXML(lang, lang->getName());
        else
            LogStream::sendNotice() << "Failed to save language " << command->getName() << endl;
   }
}

DLString LanguageHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    if (!label.empty() || !titleAttribute.empty() || !command)
        return MarkupHelpArticle::getTitle(label);

    buf << "Древний язык {c";
    if (!command->getRussianName().empty())
        buf << command->getRussianName() << "{x, {c";
    buf << command->getName() << "{x";
    return buf.str();
}

void LanguageHelp::getRawText( Character *ch, ostringstream &in ) const
{
    const Language *lang = command ? command.getDynamicPointer<Language>() : 0;
    if (!lang)
        return;

    in << "%PAUSE%";
    lang->show(ch->getPC(), in);
    in << "%RESUME%";

    in << endl
       << *this;
}

/*--------------------------------------------------------------------
 * Language
 *-------------------------------------------------------------------*/
const int Language::MAX_POWER_WORLD = 200;
const int Language::MAX_POWER_DREAM = 100;

LanguageException::LanguageException( const Language &lang, const DLString &msg )
                         : Exception( lang.getName( ) + " language: " + msg )
{
}

Language::Language( ) : cat(defaultCategory.getValue(), defaultCategory.getTable())
{
}

Language::Language( const DLString &n ) : Skill( n ),
        cat(defaultCategory.getValue(), defaultCategory.getTable())
{
}

void Language::initialization( )
{
    languageManager->load( this );

    skillManager->registrate( Pointer( this ) );
    commandManager->registrate( Pointer( this ) );
}

void Language::destruction( )
{
    skillManager->unregistrate( Pointer( this ) );
    commandManager->unregistrate( Pointer( this ) );

    languageManager->unload( this );
}

CommandHelp::Pointer Language::getHelp( ) const
{

    return help;
}

const RaceLangInfo * Language::getRaceInfo( CharacterMemoryInterface *ch ) const
{
    static const DLString otherName( "other" );
    Races::const_iterator i = races.find( ch->getRace( )->getName( ) );
    
    if (i == races.end( ))
        i = races.find( otherName );

    if (i == races.end( )) {
        LogStream::sendError( ) << "No default race defined in language profile for " << getName( ) << endl;
        return NULL;
    }
    
    return &i->second;
}

const ClassLangInfo * Language::getClassInfo( CharacterMemoryInterface *ch ) const
{
    static const DLString otherName( "other" );
    Classes::const_iterator i = classes.find( ch->getProfession( )->getName( ) );
    
    if (i == classes.end( ))
        i = classes.find( otherName );

    if (i == classes.end( )) {
        LogStream::sendError( ) << "No default class defined in language profile for " << getName( ) << endl;
        return NULL;
    }
    
    return &i->second;
}

Word Language::createGlobalWord( ) const
{
    Word word;
    DLString effectName = getRandomEffectName( true );
    
    if (!effectName.empty( )) {
        word.effect = effectName;
        word.count  = findEffect( effectName )->getFrequency( );
        word.dictum = createDictum( );
        word.lang   = name;
    }

    return word;
}

Word Language::createPersonalWord( ) const
{
    Word word;
    DLString effectName = getRandomEffectName( false );
    
    if (!effectName.empty( )) {
        word.effect = effectName;
        word.count  = 2 * findEffect( effectName )->getFrequency( );
        word.dictum = createDictum( );
        word.lang   = name;
    }

    return word;
}

WordEffect::Pointer Language::findEffect( const DLString &name ) const
{
    Effects::const_iterator e = effects.find( name );

    if (e != effects.end( )) 
        return e->second;
    else
        return WordEffect::Pointer( );
}

DLString Language::getRandomEffectName( bool fGlobalOnly ) const
{
    Effects::const_iterator e;
    int summ, i, dice;
    list<pair<DLString, int> > names;
    list<pair<DLString, int> >::iterator n;

    for (summ = 0, e = effects.begin( ); e != effects.end( ); e++)
        if (!fGlobalOnly || e->second->isGlobal( )) {
            summ += e->second->getFrequency( );
            names.push_back( make_pair( e->first, e->second->getFrequency( ) ) );
        }
    
    dice = number_range( 0, summ - 1 );
    for (i = 0, n = names.begin( ); n != names.end( ); n++) {
        i += n->second;

        if (i > dice)
            return n->first;
    }

    return DLString::emptyString;
}

DLString Language::getEffectName( WordEffectPointer effect ) const
{
    Effects::const_iterator e;
    
    for (e = effects.begin( ); e != effects.end( ); e++)
        if (e->second.getPointer( ) == effect.getPointer( ))
            return e->first;

    return DLString::emptyString;
}

bool Language::isNative( PCharacter *ch ) const
{
    if (getLearned( ch ) < SKILL_ADEPT)
        return false;

    if (getRaceInfo( ch )->maximum.getValue( ) < SKILL_NATIVE)
        return false;

    return true;
}


WordContainer * Language::locateWord( Word &word, PCharacter *ch, const DLString &arg ) const
{
    if (languageManager->findWord( word, *this, arg )) {
        return languageManager;
    }
    else {
        XMLAttributeLanguage::Pointer attr;

        attr = ch->getAttributes( ).findAttr<XMLAttributeLanguage>( "language" );
        
        if (attr && attr->findWord( word, *this, arg ))
            return *attr;
    }

    return NULL;
}

/*--------------------------------------------------------------------
 * LangInfo 
 *-------------------------------------------------------------------*/

bool LangInfo::available ( ) const
{
    return level.getValue( ) < LEVEL_IMMORTAL;
}

