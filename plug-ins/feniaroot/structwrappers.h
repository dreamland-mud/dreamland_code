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
class AreaIndexData;

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
    virtual Scripting::Object *getSelf() const { return 0; }
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString filename;

protected:
    AreaIndexData * getTarget() const;
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString name;

protected:
    Liquid * getTarget() const;
};

/*----------------------------------------------------------------------
 * Wearlocation
 *----------------------------------------------------------------------*/
class Wearlocation;

class WearlocWrapper : public PluginNativeImpl<WearlocWrapper>, 
                      public NativeHandler,
                      public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<WearlocWrapper> Pointer;

    WearlocWrapper() { }
    WearlocWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    virtual Scripting::Object *getSelf() const { return 0; }
    static Scripting::Register wrap( const DLString & );

    XML_VARIABLE XMLString name;

protected:
    Wearlocation * getTarget() const;
};

/*----------------------------------------------------------------------
 * Material
 *----------------------------------------------------------------------*/
struct material_t;

class MaterialWrapper : public PluginNativeImpl<MaterialWrapper>, 
                      public NativeHandler,
                      public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<MaterialWrapper> Pointer;

    MaterialWrapper() { }
    MaterialWrapper(const DLString &);
            
    virtual void setSelf(Scripting::Object *) { }
    virtual Scripting::Object *getSelf() const { return 0; }
    static Scripting::Register wrap( const DLString &names );

    XML_VARIABLE XMLString names;

protected:
    material_t * getTarget() const;
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
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
    virtual Scripting::Object *getSelf() const { return 0; }
    XML_VARIABLE XMLString name;

protected:
    Language * getTarget() const;
};

#endif
