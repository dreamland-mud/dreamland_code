#ifndef SPELLWRAPPER_H
#define SPELLWRAPPER_H

#include "pluginwrapperimpl.h"

struct Spell;
class SpellWrapper : public PluginWrapperImpl<SpellWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<SpellWrapper> Pointer;
    
    SpellWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( Spell* );
    void checkTarget( ) const ;
    virtual void extract( bool );
    Spell *getTarget( ) const;
private:        
    Spell *target;
};

#endif 
