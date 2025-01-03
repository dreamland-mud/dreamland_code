/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>

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
#include "commandmanager.h"
#include "xmlattributerestring.h"

#include "playerwrapper.h"
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
#include "commandwrapper.h"

#include "directions.h"
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

Room *argnum2room(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return arg2room(reg);
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

int arg2number(const Register &reg, int lower, int upper)
{
    int result = reg.toNumber();

    if (lower != 0 && upper != 0) {
        if (result < lower)
            throw Scripting::Exception("Number value must be higher than " + DLString(lower));
        if (result > upper)
            throw Scripting::Exception("Number value must be lower than " + DLString(upper));
    }

    return result;
}

DefaultAffectHandler * arg2affecthandler( const Register &reg )
{
    return dynamic_cast<DefaultAffectHandler *>(
                wrapper_cast<AffectHandlerWrapper>(reg)->getTarget());;
}

WrappedCommand * arg2command(const Register &arg) 
{
    DLString cmdName = arg2string(arg);
    Command::Pointer cmd = commandManager->findExact(cmdName);

    if (!cmd)
        throw Scripting::Exception(cmdName + ": no registered command found");

    WrappedCommand *wcmd = cmd.getDynamicPointer<WrappedCommand>();
    if (!wcmd)
        throw Scripting::Exception(cmdName + ": this command is not accessible via Fenia");

    return wcmd;
}

WrappedCommand * argnum2command(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return arg2command(reg);
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

NPCharacter *argnum2mobile(const RegisterList &args, int num)
{
    Character *ch = argnum2character(args, num);
    if (!ch->is_npc())
        throw Scripting::Exception("PC found when mobile expected.");
    return ch->getNPC();
}

PCMemoryInterface * argnum2memory(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);

    if (reg.type == Register::STRING) {
        DLString playerName = argnum2string(args, num);
        PCMemoryInterface *pci = PCharacterManager::find(playerName);
        if (!pci)
            throw Scripting::Exception("Player not found.");
        return pci;
    }

    if (reg.type == Register::OBJECT) {
        auto *chWrap = reg.toHandler().getDynamicPointer<CharacterWrapper>();
        if (chWrap) {
            if (chWrap->getTarget()->is_npc())
                throw Scripting::Exception("Mobile found when PC expected.");
            return chWrap->getTarget()->getPC();
        }

        auto *playerWrap = reg.toHandler().getDynamicPointer<PlayerWrapper>();
        if (playerWrap)
            return playerWrap->getTarget();
    }

    throw Scripting::Exception("Invalid player reference");
}

::Object *argnum2item(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return arg2item(reg);
}

int arg2flag(const Register &a, const FlagTable &table)
{
    if (a.type == Register::STRING) {
        if (table.enumerated)
            return table.value( a.toString().c_str(), true );
        else
            return table.bitstring( a.toString().c_str(), true );
    }
    return a.toNumber();
}

int argnum2flag(const RegisterList &args, int num, const FlagTable &table)
{
    Register a = argnum(args, num);
    return arg2flag(a, table);
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
    Skill *skill;

    if (r.type == Register::STRING) {
        skill = skillManager->findExisting(r.toString());
        if (!skill)
            throw Scripting::Exception(r.toString() + ": skill does not exist.");

    } else if (r.type == Register::NUMBER) {
        skill = skillManager->find(r.toNumber());
        if (!skill)
            throw Scripting::Exception(r.toString() + ": invalid skill number.");

    } else {
        throw Scripting::IllegalArgumentException();
    }

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
        target->arg = a.toString();
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

/** Resolve door argument that can be either door number or direction name, return -1 for invalid doors. */
int args2door(const RegisterList &args)
{
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    return arg2door(args.front());
}

int arg2door(const Register &arg)
{
    int door;

    if (arg.type == Register::STRING)
        door = direction_lookup(arg.toString().c_str());
    else if (arg.type == Register::NUMBER)
        door = arg.toNumber();
    else 
        throw Scripting::IllegalArgumentException();
    
    return door;
}

RegList * arg2reglist(const Register &arg)
{
    if (arg.type != Register::OBJECT)
        throw Scripting::Exception("Argument is not an object");

    Scripting::Object *obj = arg.toObject();
    if (!obj || !obj->hasHandler())
        throw Scripting::Exception("Argument is not a valid object");

    RegList *list = obj->getHandler().getDynamicPointer<RegList>();
    if (!list)
        throw Scripting::Exception("Argument is not a valid List");

    return list;
}

Scripting::Closure * argnum2closure(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return reg.toFunction();
}

RegisterList argnum2registerList(const RegisterList &args, int num)
{
    RegisterList::const_iterator ai = args.begin();

    for (int counter = num; counter > 1 && ai != args.end(); counter--) {
        ai++;
    }

    if (ai == args.end())
        throw Scripting::NotEnoughArgumentsException();

    RegisterList av;
    av.assign(ai, args.end());

    return av;
}

float arg2float(const Register &reg)
{
    if (reg.type == Register::NONE)
        return 0;

    if (reg.type == Register::NUMBER)
        return reg.toNumber();

    DLString strValue = reg.toString();
    try {
        return std::stof(strValue);

    } catch (const std::exception &e) {
        throw Scripting::Exception("Invalid float value " + DLString(e.what()));
    }
}

float argnum2float(const RegisterList &args, int num)
{
    const Register &reg = argnum(args, num);
    return arg2float(reg);
}

void args2restringAttribute(const RegisterList &args, PCMemoryInterface *pci)
{
    Skill *skill = argnum2skill(args, 1);
    DLString key = argnum2string(args, 2);
    DLString objName = argnum2string(args, 3);
    DLString objShort = argnum2string(args, 4);
    DLString objLong = argnum2string(args, 5);
    DLString objExtra = argnum2string(args, 6);

    XMLAttributeRestring::Pointer attr = pci->getAttributes( ).getAttr<XMLAttributeRestring>(skill->getName());
    XMLAttributeRestring::iterator r = attr->find( key );
    if (r != attr->end( )) {
        r->second.name = objName;
        r->second.shortDescr = objShort;
        r->second.longDescr = objLong;
        r->second.description = objExtra;
    } else {
        (**attr)[key].name = objName;
        (**attr)[key].shortDescr = objShort;
        (**attr)[key].longDescr = objLong;
        (**attr)[key].description = objExtra;
    }
}