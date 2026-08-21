/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __WRAPPERMANAGERBASE_H__
#define __WRAPPERMANAGERBASE_H__

#include <map>

#include "fenia/object.h"
#include "fenia/register-decl.h"

class Object;
class Character;
class Room;
class mob_index_data;
class obj_index_data;
class AreaIndexData;
class Spell;
class AffectHandler;
class Affect;
class SkillCommand;
class WrappedCommand;
class AreaQuest;
class Behavior;
class QuestRegistratorBase;
class WordEffect;

class WrapperManagerBase : public virtual DLObject {
public:
    typedef ::Pointer<WrapperManagerBase> Pointer;
    typedef std::map<long long, Scripting::Object *> WrapperMap;

    virtual Scripting::Register getWrapper( Character * ) = 0;
    virtual Scripting::Register getWrapper( ::Object * ) = 0;
    virtual Scripting::Register getWrapper( Room * ) = 0;
    virtual Scripting::Register getWrapper( mob_index_data * ) = 0;
    virtual Scripting::Register getWrapper( obj_index_data * ) = 0;
    virtual Scripting::Register getWrapper( AreaIndexData * ) = 0;
    virtual Scripting::Register getWrapper( Spell * ) = 0;
    virtual Scripting::Register getWrapper( AffectHandler * ) = 0;
    virtual Scripting::Register getWrapper( Affect * ) = 0;
    virtual Scripting::Register getWrapper( SkillCommand * ) = 0;
    virtual Scripting::Register getWrapper( WrappedCommand * ) = 0;
    virtual Scripting::Register getWrapper( AreaQuest * ) = 0;
    virtual Scripting::Register getWrapper( Behavior * ) = 0;
    virtual Scripting::Register getWrapper( QuestRegistratorBase * ) = 0;

    virtual void linkWrapper( Character * ) = 0;
    virtual void linkWrapper( ::Object * ) = 0;
    virtual void linkWrapper( Room * ) = 0;
    virtual void linkWrapper( mob_index_data * ) = 0;
    virtual void linkWrapper( obj_index_data * ) = 0;
    virtual void linkWrapper( AreaIndexData * ) = 0;
    virtual void linkWrapper( Spell * ) = 0;
    virtual void linkWrapper( AffectHandler * ) = 0;
    virtual void linkWrapper( Affect * ) = 0;
    virtual void linkWrapper( SkillCommand * ) = 0;
    virtual void linkWrapper( WrappedCommand * ) = 0;
    virtual void linkWrapper( AreaQuest * ) = 0;
    virtual void linkWrapper( Behavior * ) = 0;
    virtual void linkWrapper( QuestRegistratorBase * ) = 0;
    // Virtual (unlike getWrapper(WordEffect*)) so the languages plugin can bind
    // effect wrappers at language-load time through the base pointer -- languages
    // load AFTER feniaroot init, so linkTargets() there is too early to stamp them.
    virtual void linkWrapper( WordEffect * ) = 0;

    /** Turn a Fenia value back into the engine object behind it.
     *
     *  The Character form has been here for years; the other two arrived when
     *  quest_core needed them. They belong on this interface and not in
     *  feniaroot's own wrap_utils.h for a linkage reason: feniaroot LIBADDs the
     *  plugins that want to unwrap, so anything of theirs that reached into
     *  feniaroot would close a cycle. Everything in core can call these. */
    virtual void getTarget( const Scripting::Register &, Character *& ) = 0;
    virtual void getTarget( const Scripting::Register &, ::Object *& ) = 0;
    virtual void getTarget( const Scripting::Register &, Room *& ) = 0;
    void markAlive(long long id);

    static WrapperMap map;
};

#endif

