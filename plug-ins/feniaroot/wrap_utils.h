/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WRAP_UTILS_H
#define WRAP_UTILS_H

#include "wrapperbase.h"
#include "xmlregister.h"
#include "register-decl.h"
#include "globalbitvector.h"
#include "stringset.h"
#include "reglist.h"

using Scripting::Register;
using Scripting::RegisterList;

class RegList;
class Object;
class Character;
class NPCharacter;
class PCharacter;
class PCMemoryInterface;
class Room;
class Wearlocation;
class Skill;
struct obj_index_data;
struct mob_index_data;
class FlagTable;
class DefaultSpell;
class SpellTarget;
class Affect;
class WrappedCommand;
class GlobalBitvector;

Register wrap( ::Object * );
Register wrap( struct obj_index_data * );
Register wrap( Character * );
Register wrap( struct mob_index_data * );
Register wrap( Room * );
Register wrap( ::Pointer<RegList> & );

const Register & get_unique_arg( const RegisterList & );

int args2number( const RegisterList& );
DLString args2string( const RegisterList& );
DLString args2word( const RegisterList &args );
void args2buf(const RegisterList &args, char *buf, size_t bufsize);
int args2door(const RegisterList &args);

Wearlocation * arg2wearloc( const Register & );
::Object * arg2item( const Register & );
Room * arg2room( const Register & );
Character * arg2character( const Register & );
PCharacter * arg2player( const Register & );
Character * args2character( const RegisterList & );
PCharacter * args2player( const RegisterList & );
NPCharacter *argnum2mobile(const RegisterList &args, int num);
Skill * args2skill( const RegisterList & );
Skill * arg2skill( const Register & );
Affect * args2affect(const RegisterList &);
DefaultSpell * arg2spell( const Register &reg );
DLString arg2string(const Register &reg);
int arg2number(const Register &reg, int lower = 0, int upper = 0);
int arg2door(const Register &reg);
WrappedCommand * arg2command(const Register &arg);
WrappedCommand * argnum2command(const RegisterList &args, int num);

const Register & argnum(const RegisterList &args, int num);
const Register & argnum2function(const RegisterList &args, int num);
RegisterList argnum2list(const RegisterList &args, int num);
Character *argnum2character(const RegisterList &args, int num);
PCharacter *argnum2player(const RegisterList &args, int num);
PCMemoryInterface * argnum2memory(const RegisterList &args, int num);
::Object *argnum2item(const RegisterList &args, int num);
int argnum2number(const RegisterList &args, int num);
int argnum2boolean(const RegisterList &args, int num);
DLString argnum2string(const RegisterList &args, int num);
Skill * argnum2skill(const RegisterList &args, int num);
int arg2flag(const Register &arg, const FlagTable &table);
int argnum2flag(const RegisterList &args, int num, const FlagTable &table);
const FlagTable * arg2table(const Register &);
const FlagTable * argnum2table(const RegisterList &args, int num);
::Pointer<SpellTarget> arg2target(const Register &arg);
::Pointer<SpellTarget> argnum2target(const RegisterList &args, int num);
RegList * arg2reglist(const Register &arg);

template<typename Elem> inline
void arg2globalBitvector(const Register &arg, GlobalBitvector &field)
{
    GlobalRegistry<Elem> *registry = static_cast<GlobalRegistry<Elem>*>(field.getRegistry());
    GlobalBitvector newField(registry);
    
    StringSet values;

    if (arg.type == Register::OBJECT) {
        RegList *regList = arg2reglist(arg);
        for (auto &reg: *regList)
            values.insert(reg.toString());

    } else if (arg.type == Register::STRING) {
        values.fromString(arg.toString());

    } else if (arg.type == Register::NONE) {
        field.clear();
        return;

    } else {
        throw Scripting::Exception("Invalid value for global bitvector");
    }

    for (auto &v: values) {
        Elem *elem = registry->findExisting(v);
        if (!elem)
           throw Scripting::Exception("Global element not found for " + v);

        newField.set(elem->getIndex());
    }

    field.clear();
    field.set(newField);
}


#endif

