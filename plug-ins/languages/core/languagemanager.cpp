/* $Id: languagemanager.cpp,v 1.1.2.8 2009/09/20 20:14:43 rufina Exp $
 *
 * ruffina, 2005
 */
#include "logstream.h"
#include "languagemanager.h"
#include "language.h"
#include "xmlattributelanguage.h"
#include "word.h"

#include "dlscheduler.h"
#include "pcharacter.h"

#include "dreamland.h"
#include "wiznet.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

const DLString LanguageManager::TABLE_NAME = "languages";
const DLString LanguageManager::NODE_NAME = "Language";
const DLString LanguageManager::WORDS_NAME = "words";
LanguageManager * languageManager = 0;

LanguageManager::LanguageManager( )
{
    checkDuplicate( languageManager );
    languageManager = this;
}

LanguageManager::~LanguageManager( )
{
    languageManager = 0;
}

void LanguageManager::initialization( ) 
{
    SchedulerTaskRoundPlugin::initialization( );
    getWordsFile( ).load( );
}

void LanguageManager::destruction( ) 
{
    getWordsFile( ).save( );
    SchedulerTaskRoundPlugin::destruction( );
}

XMLFile LanguageManager::getWordsFile( ) 
{
    DLFile wordsFile( DLDirectory( dreamland->getDbDir( ), TABLE_NAME ),
                      WORDS_NAME,
                      ".xml" );

    return XMLFile( wordsFile, NODE_NAME, &words );
}

void LanguageManager::before( )
{
    Languages::iterator l;
    
    for (l = langs.begin( ); l != langs.end( ); l++) {
        Word word;
        Words::iterator w;
        int cur_power, max_power;

        cur_power  = getPower( **l->second );
        max_power  = Language::MAX_POWER_WORLD; 

        try {
            int repeat = 0;
            
            while (cur_power < max_power && repeat++ < 10) {
                word = l->second->createGlobalWord( );
                
                if (addWord( word )) 
                    cur_power += word.getPower( );
            }
        }
        catch (const LanguageException &e) {
            LogStream::sendError( ) << e.what( ) << endl;
        }
    }
}

void LanguageManager::run( PCharacter *ch )
{
    XMLAttributeLanguage::Pointer attr;
    time_t now;
    Language::Pointer lang;
    Word word;

    if (!( lang = getNativeLanguage( ch ) ))
        return;

    attr = ch->getAttributes( ).getAttr<XMLAttributeLanguage>( "language" );
    
    if (ch->position != POS_SLEEPING) {
        attr->sleepTime = 0;
        return;
    }
    
    now = dreamland->getCurrentTime( );
    
    if (now - attr->lastDreamTime < 24 * 60 * 60) 
        return;
    
    if (attr->getPower( **lang ) >= Language::MAX_POWER_DREAM)
        return;

    if (IS_AFFECTED( ch, AFF_SLEEP))
        return;

    if (attr->sleepTime == 0) {
        attr->sleepTime = now;
        return;
    }
    
    if (now - attr->sleepTime < number_range( 4, 10 ))
        return;
  
    word = lang->createPersonalWord( );

    if (!attr->addWord( word ))
        return;
    
    attr->lastDreamTime = now; 
     
    lang->dream( word, ch );
    wiznet( WIZ_LANGUAGE, 0, 0, "%^C3 снится слово '%s' (%s).", ch, word.toStr( ), word.effect.getValue( ).c_str( ) );
}

void LanguageManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 4, Pointer( this ) );

    getWordsFile( ).save( );
}

DLString LanguageManager::getTableName( ) const
{
    return TABLE_NAME;
}

DLString LanguageManager::getNodeName( ) const
{
    return NODE_NAME;
}

void LanguageManager::load( LanguagePointer lang )
{
    DLString name = lang->getName( );

    loadXML( *lang, name );
    langs[name] = lang;
}

void LanguageManager::unload( LanguagePointer lang ) 
{
    DLString name = lang->getName( );
    Languages::iterator ipos = langs.find( name );
    
//    saveXML( *lang, name );
    langs.erase( ipos );
}

LanguagePointer LanguageManager::findLanguage( const DLString &name )
{
    Languages::iterator ipos = langs.find( name );

    if (ipos == langs.end( ))
        return LanguagePointer( );
    else
        return ipos->second;
}

void LanguageManager::wordUsed( const Word &word, PCharacter *ch )
{
    Words::iterator w = words.find( word.dictum );
    
    if (--w->second.count > 0)
        return;

    ch->pecho( "{wСлово {w%s{w утрачивает силу.{x", 
               w->second.dictum.getValue( ).c_str( ) );

    words.erase( w );

    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );
    if (attrHints)
        attrHints->hints.erase( word.dictum );
}    


void LanguageManager::getRandomWord( Word &word, PCharacter *ch ) const
{
    Language::Pointer language = getRandomLanguage( ch );

    getRandomWord( word, language );
    if (word.empty( ))
        return;

    XMLAttributeLanguageHints::Pointer attr = ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" );
    attr->addWord( word, language->getLearned( ch ) == Language::SKILL_NATIVE );
}

void LanguageManager::getRandomWord( Word &word, Language::Pointer lang ) const
{
    Words::const_iterator w;
    int count;
    
    if (!lang)
        return;

    for (count = 0, w = words.begin( ); w != words.end( ); w++) {
        if (w->second.lang.getValue( ) != lang->getName( ))
            continue;
        
        if (number_range( 0, count++ ) == 0) 
            word = w->second;
    }
}

Language::Pointer LanguageManager::getRandomLanguage( PCharacter *ch ) const
{
    Language::Pointer lang;
    Languages::const_iterator l;
    int count;

    for (count = 0, l = langs.begin( ); l != langs.end( ); l++) 
        if (l->second->usable( ch, false ))
            if (number_range( 0, count++ ) == 0) 
                lang = l->second;
    
    return lang;
}

Language::Pointer LanguageManager::getNativeLanguage( PCharacter *ch ) const
{
    Language::Pointer lang;
    Languages::const_iterator l;
    int count;

    for (count = 0, l = langs.begin( ); l != langs.end( ); l++)
        if (l->second->isNative( ch )) 
            if (number_range( 0, count++ ) == 0) 
                lang = l->second;
    
    return lang;
}
