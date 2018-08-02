/* $Id: languagemanager.h,v 1.1.2.4 2009/09/20 20:14:43 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __LANGUAGEMANAGER_H__
#define __LANGUAGEMANAGER_H__

#include "xmlmap.h"
#include "oneallocate.h"
#include "xmlfile.h"
#include "dlxmlloader.h"
#include "schedulertaskroundplugin.h"
#include "schedulertaskroundpcharacter.h"
#include "word.h"

class Language;

typedef ::Pointer<Language> LanguagePointer;


class LanguageManager : public WordContainer, 
                        public SchedulerTaskRoundPlugin, 
			public virtual SchedulerTaskRoundPCharacter, 
                        public OneAllocate, 
			public DLXMLLoader 
{
public:
    typedef ::Pointer<LanguageManager> Pointer;
    typedef std::map<DLString, LanguagePointer> Languages;
    
    LanguageManager( );
    virtual ~LanguageManager( );
    
    virtual void initialization( );
    virtual void destruction( );

    virtual void run( PCharacter * );
    virtual void before( );
    virtual void after( );

    void load( LanguagePointer );
    void unload( LanguagePointer );
    LanguagePointer findLanguage( const DLString & );
    
    virtual DLString getTableName( ) const;
    virtual DLString getNodeName( ) const;
    
    virtual void wordUsed( const Word &, PCharacter * );

    void getRandomWord( Word &, PCharacter * ) const;
    
private:
    void getRandomWord( Word &, LanguagePointer ) const;
    LanguagePointer getRandomLanguage( PCharacter * ) const;
    LanguagePointer getNativeLanguage( PCharacter * ) const;
    XMLFile getWordsFile( );
    
    Languages langs;

    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;
    static const DLString WORDS_NAME;
};

extern LanguageManager * languageManager;

#endif
