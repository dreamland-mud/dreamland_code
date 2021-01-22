#ifndef SPELLWRAPPER_H
#define SPELLWRAPPER_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
// MOC_SKIP_BEGIN
#include "exceptions.h"
// MOC_SKIP_END
#include "fenia/handler.h"
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
