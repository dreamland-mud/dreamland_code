#ifndef AREAQUESTWRAPPER_H 
#define AREAQUESTWRAPPER_H 

#include "pluginwrapperimpl.h"

class AreaQuest;

/** A Fenia wrapper around area quests, allowing to add custom
 * field and methods to a quest from a Fenia scenario (such as onInit, onReward etc).
 * Also provides access to native field/methods (step list, title, vnum etc).
 */
class AreaQuestWrapper : public PluginWrapperImpl<AreaQuestWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<AreaQuestWrapper> Pointer;
    
    AreaQuestWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( AreaQuest* );
    void checkTarget( ) const ;
    virtual void extract( bool );
    AreaQuest *getTarget( ) const;

private:        
    AreaQuest *target;
};

#endif 
