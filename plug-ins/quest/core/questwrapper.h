/* Dream Land, 2026 */
#ifndef QUESTWRAPPER_H
#define QUESTWRAPPER_H

#include "xmlvariablecontainer.h"
#include "fenia/handler.h"
#include "pluginnativeimpl.h"
#include "quest.h"

class FeniaQuest;

using Scripting::NativeHandler;

/** A player's quest, as Fenia sees it: the state bag a scenario reads and
 *  writes while it runs.
 *
 *  Short-lived on purpose. One is built per call into a Fenia method and thrown
 *  away after, so it is nothing like .AutoQuest() -- that wrapper IS the type and
 *  lives in the Fenia DB, this one only points at a quest somebody currently
 *  holds. It keeps a counted pointer rather than a raw one so a script that
 *  stashes it somewhere gets an error instead of a crash; after a reboot the
 *  target is gone and every accessor says so.
 */
class QuestWrapper : public PluginNativeImpl<QuestWrapper>,
                     public NativeHandler,
                     public XMLVariableContainer
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<QuestWrapper> Pointer;

    QuestWrapper( );
    QuestWrapper( FeniaQuest * );

    virtual void setSelf( Scripting::Object *s ) { self = s; }
    virtual Scripting::Object *getSelf( ) const { return self; }

    static Scripting::Register wrap( FeniaQuest * );

protected:
    FeniaQuest *getTarget( );

    ::Pointer<FeniaQuest> target;
    Scripting::Object *self;
};

/** The reward a Fenia onReward fills in.
 *
 *  Handed to the script as an argument to be written into, rather than expecting
 *  a Map back: a Map swallows a misspelled key without a word, and a quest that
 *  quietly pays nothing is a bug report six months later. Every field here is
 *  named, and an unknown one is an error at the point of writing.
 */
class QuestRewardWrapper : public PluginNativeImpl<QuestRewardWrapper>,
                           public NativeHandler,
                           public XMLVariableContainer
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<QuestRewardWrapper> Pointer;

    QuestRewardWrapper( );
    QuestRewardWrapper( QuestReward::Pointer & );

    virtual void setSelf( Scripting::Object *s ) { self = s; }
    virtual Scripting::Object *getSelf( ) const { return self; }

    static Scripting::Register wrap( QuestReward::Pointer & );

protected:
    QuestReward *getTarget( );

    QuestReward::Pointer target;
    Scripting::Object *self;
};

#endif
