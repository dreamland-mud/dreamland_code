/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __STRUCTWRAPPERS_H__
#define __STRUCTWRAPPERS_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
#include "pluginwrapperimpl.h"
#include "basicskill.h"

using Scripting::NativeHandler;

/*----------------------------------------------------------------------
 * Area
 *----------------------------------------------------------------------*/
class Area;

class AreaWrapper : public PluginNativeImpl<AreaWrapper>, 
                        public NativeHandler,
                        public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<AreaWrapper> Pointer;

    AreaWrapper() { }
    AreaWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString filename;
};

/*----------------------------------------------------------------------
 * Hometown
 *----------------------------------------------------------------------*/
class Hometown;

class HometownWrapper : public PluginNativeImpl<HometownWrapper>, 
                        public NativeHandler,
                        public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<HometownWrapper> Pointer;

    HometownWrapper() { }
    HometownWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString name;
};

/*----------------------------------------------------------------------
 * Profession
 *----------------------------------------------------------------------*/
class Profession;

class ProfessionWrapper : public PluginNativeImpl<ProfessionWrapper>, 
                          public NativeHandler,
                          public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<ProfessionWrapper> Pointer;

    ProfessionWrapper() { }
    ProfessionWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    XML_VARIABLE XMLString name;
};

/*----------------------------------------------------------------------
 * CraftProfession
 *----------------------------------------------------------------------*/
class CraftProfession;

class CraftProfessionWrapper : public PluginNativeImpl<CraftProfessionWrapper>, 
                          public NativeHandler,
                          public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<CraftProfessionWrapper> Pointer;

    CraftProfessionWrapper() { }
    CraftProfessionWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    XML_VARIABLE XMLString name;

protected:
    CraftProfession * getTarget() const;
};

/*----------------------------------------------------------------------
 * Bonus
 *----------------------------------------------------------------------*/
class Bonus;

class BonusWrapper : public PluginNativeImpl<BonusWrapper>, 
                          public NativeHandler,
                          public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<BonusWrapper> Pointer;

    BonusWrapper() { }
    BonusWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    XML_VARIABLE XMLString name;

protected:
    Bonus * getTarget() const;
};

/*----------------------------------------------------------------------
 * Race
 *----------------------------------------------------------------------*/
class Race;

class RaceWrapper : public PluginNativeImpl<RaceWrapper>, 
                    public NativeHandler,
                    public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<RaceWrapper> Pointer;

    RaceWrapper() { }
    RaceWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString name;
};

/*----------------------------------------------------------------------
 * Liquid
 *----------------------------------------------------------------------*/
class Liquid;

class LiquidWrapper : public PluginNativeImpl<LiquidWrapper>, 
                      public NativeHandler,
                      public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<LiquidWrapper> Pointer;

    LiquidWrapper() { }
    LiquidWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString name;

protected:
    Liquid * getTarget() const;
};

/*----------------------------------------------------------------------
 * Clan
 *----------------------------------------------------------------------*/
class Clan;

class ClanWrapper : public PluginNativeImpl<ClanWrapper>, 
                    public NativeHandler,
                    public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<ClanWrapper> Pointer;

    ClanWrapper() { }
    ClanWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString name;
};

/*----------------------------------------------------------------------
 * Religion
 *----------------------------------------------------------------------*/
class DefaultReligion;

class ReligionWrapper : public PluginNativeImpl<ReligionWrapper>, 
                    public NativeHandler,
                    public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<ReligionWrapper> Pointer;

    ReligionWrapper() { }
    ReligionWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    XML_VARIABLE XMLString name;

protected:
    DefaultReligion * getTarget() const;
};

/*----------------------------------------------------------------------
 * Language
 *----------------------------------------------------------------------*/
class Language;

class LanguageWrapper : public PluginNativeImpl<LanguageWrapper>, 
                    public NativeHandler,
                    public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<LanguageWrapper> Pointer;

    LanguageWrapper() { }
    LanguageWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    
    XML_VARIABLE XMLString name;

protected:
    Language * getTarget() const;
};

/*----------------------------------------------------------------------
 * Skill
 *----------------------------------------------------------------------*/
class Skill;

class SkillWrapper : public PluginNativeImpl<SkillWrapper>, 
                      public NativeHandler,
                      public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<SkillWrapper> Pointer;

    SkillWrapper() { }
    SkillWrapper(const DLString &);            
    virtual void setSelf(Scripting::Object *) { }
    Skill * getTarget() const;
    
protected:
    XML_VARIABLE XMLString name;
};

class FeniaSkill : public PluginNativeImpl<FeniaSkill>, 
                       public NativeHandler,
                       public BasicSkill
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<FeniaSkill> Pointer;

    FeniaSkill() { }
    FeniaSkill(const DLString &);

    virtual void setSelf(Scripting::Object *);
    static Scripting::Register wrap( const DLString & );
    virtual void backup();
private:
    Scripting::Object *self;
    XML_VARIABLE XMLString myname;
};

#endif
