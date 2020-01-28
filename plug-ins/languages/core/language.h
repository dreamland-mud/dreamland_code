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

#include "skill.h"
#include "affecthandler.h"
#include "spell.h"
#include "command.h"
#include "commandhelp.h"
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


class Language : public Skill, public Command, public virtual Plugin, public XMLVariableContainer {
XML_OBJECT
friend class LanguageManager;
public:
    typedef ::Pointer<Language> Pointer;
    typedef ::Pointer<WordEffect> WordEffectPointer;
    typedef XMLMapBase<RaceLangInfo> Races;
    typedef XMLMapBase<ClassLangInfo> Classes;
    typedef XMLVectorBase<XMLString> WordList;
    typedef XMLMapBase<XMLString> WordMap;
    typedef XMLMapBase<XMLPointer<WordEffect> > Effects;
    
    Language( );
    Language( const DLString & );

    virtual void initialization( );
    virtual void destruction( );
    
    virtual const DLString & getName( ) const;
    virtual const Enumeration & getPosition( ) const;
    virtual const Flags & getCommandCategory( ) const;
    virtual const DLString & getRussianName( ) const;
    virtual const DLString & getHint( ) const;
    virtual void run( Character *, const DLString & );
    
    virtual AffectHandler::Pointer getAffect( );
    virtual Spell::Pointer getSpell( ) const;
    virtual CommandHelp::Pointer getHelp( ) const;
    virtual int getBeats( ) const;
    virtual int getMana( ) const;
    virtual SkillGroupReference & getGroup( );
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    virtual int getWeight( Character * ) const;
    virtual int getMaximum( Character * ) const;
    virtual int getAdept( PCharacter * ) const;
    virtual bool canForget( PCharacter * ) const;
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool );
    virtual void practice( PCharacter * ) const;
    virtual void show( PCharacter *, std::ostream & ); 
    virtual void improve( Character *, bool, Character *victim = NULL, int dam_type = -1, int dam_flags = 0 ) const;
    virtual const DLString & getCategory( ) const
    {
        return CATEGORY;
    }
    
    WordEffectPointer findEffect( const DLString & ) const;
    DLString getEffectName( WordEffectPointer ) const;
    bool isNative( PCharacter * ) const;
    virtual DLString createDictum( ) const = 0;
    
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

protected:
    Word createGlobalWord( ) const;
    Word createPersonalWord( ) const;
    DLString getRandomEffectName( bool ) const;
    virtual void dream( const Word &, PCharacter * ) const = 0;
    const RaceLangInfo * getRaceInfo( PCharacter * ) const;
    const ClassLangInfo * getClassInfo( PCharacter * ) const;
    WordContainer * locateWord( Word &, PCharacter *, const DLString & ) const;

    static const DLString CATEGORY;

    XML_VARIABLE XMLString  nameRus;
    XML_VARIABLE XMLStringNoEmpty nameRusNoCase;
    XML_VARIABLE XMLString  hint;
    XML_VARIABLE XMLPointerNoEmpty<CommandHelp> help;
    XML_VARIABLE XMLInteger beats;
    XML_VARIABLE XMLInteger minAlign, maxAlign;
    XML_VARIABLE Races    races;
    XML_VARIABLE Classes  classes;
    XML_VARIABLE Effects  effects;
    XML_VARIABLE XMLFlags cat;

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
