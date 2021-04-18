#ifndef SKILLCOMMANDWRAPPER_H
#define SKILLCOMMANDWRAPPER_H

#include "pluginwrapperimpl.h"

struct SkillCommand;

class SkillCommandWrapper : public PluginWrapperImpl<SkillCommandWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<SkillCommandWrapper> Pointer;
    
    SkillCommandWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( SkillCommand* );
    void checkTarget( ) const ;
    virtual void extract( bool );
    SkillCommand *getTarget( ) const;

private:        
    SkillCommand *target;
};

#endif 
