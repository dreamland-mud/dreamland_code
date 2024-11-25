#ifndef SKILLWRAPPER_H
#define SKILLWRAPPER_H

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
 * Skill
 *----------------------------------------------------------------------*/
class Skill;
class SkillGroup;
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
    virtual Scripting::Object *getSelf() const { return 0; }
    Skill * getTarget() const;
    
protected:
    XML_VARIABLE XMLString name;
};


/*----------------------------------------------------------------------
 * SkillGroup
 *----------------------------------------------------------------------*/

class SkillGroupWrapper : public PluginNativeImpl<SkillGroupWrapper>, 
                      public NativeHandler,
                      public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<SkillGroupWrapper> Pointer;

    SkillGroupWrapper() { }
    SkillGroupWrapper(const DLString &);

    virtual void setSelf(Scripting::Object *) { }
    virtual Scripting::Object *getSelf() const { return 0; }
    SkillGroup * getTarget() const;
    static Scripting::Register wrap(const DLString &);

protected:
    XML_VARIABLE XMLString name;
};


#endif