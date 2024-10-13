/* $Id: language.h,v 1.1.2.11 2010-08-24 20:33:31 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#include "exception.h"
#include "enumeration.h"
#include "xmlmap.h"
#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlpointer.h"
#include "xmlflags.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "skill.h"
#include "affecthandler.h"
#include "spell.h"
#include "command.h"
#include "plugin.h"

class LangInfo;
typedef LangInfo RaceLangInfo;
typedef LangInfo ClassLangInfo;
class Word;
class WordContainer;
class WordEffect;
class Language;

class LanguageException : public Exception {
public:
    LanguageException( const Language &, const DLString & );
};

class LanguageHelp : public CommandHelp {
public:
    typedef ::Pointer<LanguageHelp> Pointer;

    virtual DLString getTitle(const DLString &label) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    virtual void getRawText( Character *, ostringstream & ) const;
};

inline const DLString & LanguageHelp::getType( ) const
{
    return TYPE;
}

class LanguageCommand : public Command {
XML_OBJECT
public:
    typedef ::Pointer<LanguageCommand> Pointer;

    virtual void run( Character *, const DLString & );
    virtual bool saveCommand() const;

    void setLanguage(::Pointer<Language> language);
    void unsetLanguage();
    ::Pointer<Language> getLanguage() const;

protected:    
    void doUtter( PCharacter *, DLString &, DLString & ) const;
    void doInit( PCharacter *, DLString & ) const;
    void doList( PCharacter * ) const;
    void doKnown( PCharacter * ) const;
    bool showDreams( PCharacter * ) const;
    bool showRewards( PCharacter * ) const;
    void doIdent( PCharacter *, DLString & ) const;
    void doForget( PCharacter *, const DLString & ) const;
    void doRemember( PCharacter *, const DLString & ) const;

    ::Pointer<Language> language;
};

class Language : public virtual Skill, public virtual Plugin, public XMLVariableContainer {
XML_OBJECT
friend class LanguageManager;
friend class LanguageCommand;
public:
    typedef ::Pointer<Language> Pointer;
    typedef ::Pointer<WordEffect> WordEffectPointer;
    typedef XMLMapBase<RaceLangInfo> Races;
    typedef XMLMapBase<ClassLangInfo> Classes;
    typedef XMLVectorBase<XMLString> WordList;
    typedef XMLMapBase<XMLString> WordMap;
    typedef XMLMapBase<XMLPointer<WordEffect> > Effects;
    
    Language( const DLString & );

    virtual void initialization( );
    virtual void destruction( );
    
    virtual const DLString & getName( ) const;
    virtual const DLString & getRussianName( ) const;
    virtual bool matchesStrict( const DLString &str ) const;
    virtual bool matchesUnstrict( const DLString &str ) const;
    virtual bool matchesSubstring( const DLString &str ) const;
    virtual const DLString& getNameFor( Character * ) const;
    virtual AffectHandler::Pointer getAffect( );
    virtual Spell::Pointer getSpell( ) const;
    virtual HelpArticlePointer getSkillHelp( ) const;
    virtual int getBeats(Character *ch = 0) const;
    virtual int getMana( ) const;
    virtual GlobalBitvector & getGroups();
    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    virtual int getMaximum( Character * ) const;
    virtual int getAdept( PCharacter * ) const;
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );
    virtual void practice( PCharacter * ) const;
    virtual void show( PCharacter *, std::ostream & ) const; 
    virtual void improve( Character *, bool, Character *victim = NULL, int dam_type = -1, int dam_flags = 0 ) const;
    virtual const DLString & getCategory( ) const
    {
        return CATEGORY;
    }
    virtual bool isValid( ) const
    {
        return true;
    }
    
    WordEffectPointer findEffect( const DLString & ) const;
    DLString getEffectName( WordEffectPointer ) const;
    bool isNative( PCharacter * ) const;
    virtual DLString createDictum( ) const = 0;
    
protected:
    Word createGlobalWord( ) const;
    Word createPersonalWord( ) const;
    DLString getRandomEffectName( bool ) const;
    virtual void dream( const Word &, PCharacter * ) const = 0;
    const RaceLangInfo * getRaceInfo( CharacterMemoryInterface * ) const;
    const ClassLangInfo * getClassInfo( CharacterMemoryInterface * ) const;
    WordContainer * locateWord( Word &, PCharacter *, const DLString & ) const;

    static const DLString CATEGORY;

    XML_VARIABLE XMLPointerNoEmpty<LanguageCommand> command;
    XML_VARIABLE XMLMultiString name;
    XML_VARIABLE XMLInteger beats;
    XML_VARIABLE XMLInteger minAlign, maxAlign;
    XML_VARIABLE Races    races;
    XML_VARIABLE Classes  classes;
    XML_VARIABLE Effects  effects;

    static const int SKILL_ADEPT;
    static const int SKILL_SENSE;
    static const int SKILL_NATIVE;
    static const int MAX_POWER_WORLD;
    static const int MAX_POWER_DREAM;
    static const Enumeration defaultPosition;
};


class LangInfo : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<LangInfo> Pointer;
    
    bool available ( ) const;

    inline int getLevel( ) const {
        return level.getValue( );
    }
    inline int getMaximum( ) const {
        return maximum.getValue( );
    }
    
    XML_VARIABLE XMLInteger maximum;
    XML_VARIABLE XMLInteger level;
};


#endif
