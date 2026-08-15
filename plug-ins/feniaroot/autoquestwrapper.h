#ifndef AUTOQUESTWRAPPER_H
#define AUTOQUESTWRAPPER_H

#include "pluginwrapperimpl.h"

class QuestRegistratorBase;

/** A Fenia wrapper around one autoquest TYPE (kill, steal, kidnap...), so that
 * type's scenario logic can be written and hot-reloaded in Fenia instead of
 * compiled into the plugin. Also gives read access to the type's config.
 *
 * The target is the registrator, not a quest in flight: there is one per type,
 * it lives for the whole run, and it is what C++ dispatches through. A player's
 * own quest is a separate, short-lived object.
 */
class AutoQuestWrapper : public PluginWrapperImpl<AutoQuestWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<AutoQuestWrapper> Pointer;

    AutoQuestWrapper( );

    virtual void setSelf( Scripting::Object * );
    void setTarget( QuestRegistratorBase * );
    void checkTarget( ) const;
    virtual void extract( bool );
    QuestRegistratorBase *getTarget( ) const;

private:
    QuestRegistratorBase *target;
};

#endif
