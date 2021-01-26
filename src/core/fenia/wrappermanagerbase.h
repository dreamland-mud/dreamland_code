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

    virtual void linkWrapper( Character * ) = 0;
    virtual void linkWrapper( ::Object * ) = 0;
    virtual void linkWrapper( Room * ) = 0;
    virtual void linkWrapper( mob_index_data * ) = 0;
    virtual void linkWrapper( obj_index_data * ) = 0;
    virtual void linkWrapper( AreaIndexData * ) = 0;
    virtual void linkWrapper( Spell * ) = 0;
    
    virtual void getTarget( const Scripting::Register &, Character *& ) = 0;
    void markAlive(long long id);

    static WrapperMap map;
};

#endif

