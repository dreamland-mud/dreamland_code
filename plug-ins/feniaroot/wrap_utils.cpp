/* $Id$
 *
 * ruffina, 2004
 */
#include "wrap_utils.h"
#include "logstream.h"

#include "affect.h"
#include "pcharacter.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "room.h"
#include "pcharactermanager.h"
#include "skill.h" 
#include "skillmanager.h" 
#include "defaultspell.h"
#include "defaultaffecthandler.h"
#include "spelltarget.h"

#include "tableswrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "mobindexwrapper.h"
#include "structwrappers.h"
#include "affectwrapper.h"
#include "spellwrapper.h"
#include "affecthandlerwrapper.h"
#include "xmleditorinputhandler.h"

#include "subr.h"
#include "def.h"

Register wrap( ::Object * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( OBJ_INDEX_DATA * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( Character * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( MOB_INDEX_DATA * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( Room * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}

Register wrap( RegList::Pointer &list )
{
    Scripting::Object *listObj = &Scripting::Object::manager->allocate();
    listObj->setHandler(list);
    return Register(listObj);
}

const Register & get_unique_arg( const RegisterList &args )
{
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    return args.front( );
}

int args2number( const RegisterList &args )
{
    return get_unique_arg( args ).toNumber( );
}

DLString args2string( const RegisterList &args )
{
    return get_unique_arg( args ).toString( );
}

// Return unquoted arguments, for methods that accept both [abc def] and ['abc def'].
DLString args2word( const RegisterList &args )
{
    DLString word = get_unique_arg(args).toString();
    return word.substitute('\'', "");
}

Character * args2character( const RegisterList &args )
{
    return arg2character(get_unique_arg(args));
}

PCharacter * args2player( const RegisterList &args )
{
    Character *ch = args2character(args); 
    if (ch->is_npc())
        throw Scripting::Exception("Mobile found when PC expected.");
    return ch->getPC();
}

Wearlocation * arg2wearloc( const Register &reg )
{
    DLString locName = reg.toString( );
    Wearlocation *loc = wearlocationManager->findExisting( locName );

    if (!loc)
        throw Scripting::Exception( DLString("Invalid wearlocation name '") + locName + "'" );

    return loc;
}

::Object * arg2item( const Register &reg )
{
    return wrapper_cast<ObjectWrapper>( reg )->getTarget( );
}

Room * arg2room( const Register &reg )
{
    return wrapper_cast<RoomWrapper>( reg )->getTarget( );
}

Character * arg2character( const Register &reg )
{
    return wrapper_cast<CharacterWrapper>( reg )->getTarget( );
}

PCharacter * arg2player( const Register &reg )
{
    Character *ch = arg2character(reg);
    if (ch->is_npc())
        throw Scripting::Exception("Mobile found when PC expected.");
    return ch->getPC();
}

DefaultSpell * arg2spell( const Register &reg )
{
    return dynamic_cast<DefaultSpell *>(
                wrapper_cast<SpellWrapper>(reg)->getTarget());;
}

DLString arg2string(const Register &reg) 
{
    if (reg.type != Register::STRING)
        throw Scripting::InvalidCastException("string", reg.getTypeName());
    
    return reg.toString();
}

DefaultAffectHandler * arg2affecthandler( const Register &reg )
{
    return dynamic_cast<DefaultAffectHandler *>(
                wrapper_cast<AffectHandlerWrapper>(reg)->getTarget());;
}

void args2buf(const RegisterList &args, char *buf, size_t bufsize)
{
    strncpy(buf, args2string(args).c_str(), bufsize);
    buf[bufsize - 1] = 0;
}

static RegisterList::const_iterator argnum2iterator(const RegisterList &args, int num)
{
    if (args.size() < (unsigned int)num)
        throw Scripting::NotEnoughArgumentsException();
    
    RegisterList::const_iterator a;
    int i;
    for (a = args.begin(), i = 1; i <= num - 1 && a != args.end(); i++, a++) 
        ; 
    return a;
}

const Register & argnum(const RegisterList &args, int num)
{
    RegisterList::const_iterator a = argnum2iterator(args, num);
    return *a;
}

const Register & argnum2function(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    if (reg.type != Register::FUNCTION)
        throw Scripting::IllegalArgumentException();
    return reg;
}

RegisterList argnum2list(const RegisterList &args, int num)
{
    RegisterList::const_iterator a = argnum2iterator(args, num);
    RegisterList result;
    result.assign(a, args.end());
    return result;
}

Character *argnum2character(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return arg2character(reg);
}

PCharacter *argnum2player(const RegisterList &args, int num)
{
    Character *ch = argnum2character(args, num);
    if (ch->is_npc())
        throw Scripting::Exception("Mobile found when PC expected.");
    return ch->getPC();
}

PCMemoryInterface * argnum2memory(const RegisterList &args, int num)
{
    DLString playerName = argnum2string(args, num);
    PCMemoryInterface *pci = PCharacterManager::find(playerName);
    if (!pci)
        throw Scripting::Exception("Player not found.");
    return pci;
}

::Object *argnum2item(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return arg2item(reg);
}


int argnum2flag(const RegisterList &args, int num, const FlagTable &table)
{
    Register a = argnum(args, num);
    if (a.type == Register::STRING) {
        if (table.enumerated)
            return table.value( a.toString().c_str(), true );
        else
            return table.bitstring( a.toString().c_str(), true );
    }
    return a.toNumber();
}

int argnum2number(const RegisterList &args, int num)
{
    return argnum(args, num).toNumber();
}

int argnum2boolean(const RegisterList &args, int num)
{
    return argnum(args, num).toBoolean();
}

DLString argnum2string(const RegisterList &args, int num)
{
    return argnum(args, num).toString();
}

Skill * arg2skill( const Register &r )
{
    DLString name = r.toString();
    Skill *skill = skillManager->findExisting(name);
    if (!skill)
        throw Scripting::Exception(name + ": skill name not found.");

    return skill;
}

Skill * args2skill( const RegisterList &args )
{
    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );

    return arg2skill(args.front());
}

Skill * argnum2skill(const RegisterList &args, int num)
{
    return arg2skill(argnum(args, num));
}

SpellTarget::Pointer arg2target(const Register &a)
{
    SpellTarget::Pointer target(NEW);

    switch (a.type) {
    case Register::STRING:
    case Register::NUMBER:
        target->type = SpellTarget::NONE;
        target->arg = a.toString().c_str();
        return target;

    case Register::OBJECT:
        {
            CharacterWrapper *chWrap = a.toHandler().getDynamicPointer<CharacterWrapper>();
            if (chWrap) {
                target->type = SpellTarget::CHAR;
                target->victim = chWrap->getTarget();
                return target;
            }

            ObjectWrapper *objWrap = a.toHandler().getDynamicPointer<ObjectWrapper>(); 
            if (objWrap) {
                target->type = SpellTarget::OBJECT;
                target->obj = objWrap->getTarget();
                return target;
            }

            RoomWrapper *roomWrap = a.toHandler().getDynamicPointer<RoomWrapper>(); 
            if (roomWrap) {
                target->type = SpellTarget::ROOM;
                target->room = roomWrap->getTarget();
                return target;
            }

            /* FALLTHROUGH */
        } 
    
    default:
        return target;
    }
}

SpellTarget::Pointer argnum2target(const RegisterList &args, int num)
{
    return arg2target(argnum(args, num));
}

Affect * args2affect(const RegisterList &args)
{
    AffectWrapper *aw;

    if (args.empty())
        throw Scripting::NotEnoughArgumentsException();

    aw = wrapper_cast<AffectWrapper>(args.front());
    return &(aw->getTarget());
}

const FlagTable * arg2table(const Register &r)
{
    const FlagTable *table = 0;

    if (r.type == Register::NONE || (r.type == Register::NUMBER && r.toNumber() == 0)) {
        return 0;

    } else if (r.type == Register::STRING) {
        table = FlagTableRegistry::getTable(r.toString());

    } else if (r.type == Register::OBJECT) {
        TableWrapper *twrap = r.toHandler().getDynamicPointer<TableWrapper>();
        if (twrap)
            table = FlagTableRegistry::getTable(twrap->tableName);
        else
            throw Scripting::Exception("Argument is not a table name nor a .tables.<table> object");
    }

    if (!table)
        throw Scripting::Exception("Null or unknown flag table name.");

    return table;
}

const FlagTable * argnum2table(const RegisterList &args, int num)
{
    return arg2table(argnum(args, num));
}
