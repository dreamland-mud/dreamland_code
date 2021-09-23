#include "feniaskillaction.h"

#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "subr.h"
#include "closure.h"
#include "characterwrapper.h"
#include "objectwrapper.h"
#include "structwrappers.h"
#include "feniamanager.h"

#include "skillreference.h"
#include "skill.h"
#include "core/object.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "roomutils.h"
#include "commonattributes.h"
#include "religion.h"
#include "magic.h"
#include "damageflags.h"
#include "fight.h"
#include "fight_exception.h"
#include "effects.h"
#include "defaultspell.h"
#include "dl_math.h"
#include "../anatolia/handler.h"
#include "dreamland.h"
#include "vnum.h"
#include "merc.h"
#include "def.h"

using Scripting::NativeTraits;
using namespace Scripting;

GSN(blindness);
GSN(curse);
RELIG(none);

DLString regfmt(Character *to, const RegisterList &argv);

static RegisterList message_args(FeniaSpellContext *thiz, const RegisterList &args)
{
    RegisterList myArgs(args); 
    Register fmt = myArgs.front();
    myArgs.pop_front();

    if (thiz->vict.type != Register::NONE)
        myArgs.push_front(thiz->vict);
    else if (thiz->obj.type != Register::NONE)
        myArgs.push_front(thiz->obj);
    else if (thiz->room.type != Register::NONE)
        myArgs.push_front(thiz->room);
    else
        myArgs.push_front(thiz->arg);

    myArgs.push_front(thiz->ch);
    myArgs.push_front(fmt);
    return myArgs;
}

NMI_INVOKE(FeniaSpellContext, msgChar, "(fmt[,args]): выдать сообщение кастеру; кастер 1й аргумент, цель 2й аргумент")
{
    Character *caster = arg2character(ch);
    caster->pecho(regfmt(caster, message_args(this, args)));
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgVict, "(fmt[,args]): выдать сообщение жертве; кастер 1й аргумент, цель 2й аргумент")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *victim = arg2character(vict);
    victim->pecho(regfmt(victim, message_args(this, args)));
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgNotVict, "(fmt[,args]): выдать сообщение всем, кроме кастера и жертвы; кастер 1й аргумент, цель 2й аргумент")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *caster = arg2character(ch);
    Character *victim = arg2character(vict);
    RegisterList myArgs = message_args(this, args);

    for (Character *to = victim->in_room->people; to; to = to->next_in_room) {
        if (to == caster || to == victim)
            continue;            
        if (!to->can_sense(caster))
            continue;

        to->pecho(POS_RESTING, regfmt(to, myArgs).c_str());
    }

    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgRoom, "(fmt[,args]): выдать сообщение всем, кроме кастера; кастер 1й аргумент, цель 2й аргумент")
{
    Character *caster = arg2character(ch);
    RegisterList myArgs = message_args(this, args); 

    for (Character *to = caster->in_room->people; to; to = to->next_in_room) {
        if (to == caster)
            continue;
        if (!to->can_sense(caster))
            continue;

        to->pecho(POS_RESTING, regfmt(to, myArgs).c_str());
    }
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgAll, "(fmt[,args]): выдать сообщение всем в комнате; кастер 1й аргумент, цель 2й аргумент")
{
    Character *caster = arg2character(ch);
    RegisterList myArgs = message_args(this, args); 
    caster->in_room->echo(POS_RESTING, regfmt(NULL, myArgs).c_str());
    return Register();
}

NMI_INVOKE(FeniaSpellContext, msgArea, "(fmt[,args]): выдать сообщение всем в той же зоне, кроме кастера")
{
    Character *caster = arg2character(ch);
    RegisterList myArgs = message_args(this, args); 
    DLString message = regfmt(NULL, myArgs);
    area_message(caster, message, true);
    return Register();
}


NMI_INVOKE(FeniaSpellContext, calcDamage, "(): пересчитать повреждения согласно текущему level и tier")
{
    this->calcDamage();
    return Register(dam);
}

NMI_INVOKE(FeniaSpellContext, savesSpell, "([damtype,damflags]): уменьшить повреждения вдвое, если прошел спассбросок у жертвы; по умолчанию damtype и damflags берутся из профайла")
{
    if (vict.type == Register::NONE)
        return Register();

    DefaultSpell *mySpell = arg2spell(spell);
    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);

    bitnumber_t damtype = args.empty() ? mySpell->damtype.getValue() : argnum2flag(args, 1, damage_table);
    bitstring_t damflags = args.size() <= 1 ? mySpell->damflags : argnum2flag(args, 2, damage_flags);
    if (mySpell->isPrayer(myCh))
        damflags |= DAMF_PRAYER;
    else
        damflags |= DAMF_MAGIC;
    
    if (saves_spell(level, myVict, damtype, myCh, damflags)) {
        dam /= 2;
        return Register(true);
    }
        
    return Register(false);
}

NMI_INVOKE(FeniaSpellContext, checkDispel, "(skill): попробовать сбить с victim аффект с указанным именем")
{
    if (vict.type == Register::NONE)
        return Register(false);

    Character *myVict = arg2character(vict);
    Skill *skill = argnum2skill(args, 1);
    return Register(checkDispel(level, myVict, skill->getIndex()));
}

NMI_INVOKE(FeniaSpellContext, damage, "([damtype,damflags]): нанести повреждения жертве, по умолчанию damtype и damflags берутся из профайла")
{
    if (vict.type == Register::NONE)
        return Register();
 
    DefaultSpell *mySpell = arg2spell(spell);
    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);
    int sn = mySpell->getSkill()->getIndex();

    bitnumber_t damtype = args.empty() ? mySpell->damtype.getValue() : argnum2flag(args, 1, damage_table);
    bitstring_t damflags = args.size() <= 1 ? mySpell->damflags : argnum2flag(args, 2, damage_flags);
    if (mySpell->isPrayer(myCh))
        damflags |= DAMF_PRAYER;
    else
        damflags |= DAMF_MAGIC;

    try {
        damage_nocatch(myCh, myVict, dam, sn, damtype, true, damflags);    

    } catch (const VictimDeathException &e) {
        throw Scripting::CustomException("victim is dead");
    }

    return Register(false);
}

NMI_INVOKE(FeniaSpellContext, groupCast, "(func): вызвать ф-ию для всех согруппников в той же комнате, у кого не сработал spellbane")
{
    DefaultSpell *mySpell = arg2spell(spell);    
    Character *caster = arg2character(ch);
    RegisterList::const_iterator ai = args.begin();
    Register rfun = *ai++;
    Closure *fun = rfun.toFunction( );
    RegisterList funArgs;
    funArgs.assign(ai, args.end( ));

    Register thiz = Register(self);

    for (auto &rch: caster->in_room->getPeople()) {
        if (rch->in_room != caster->in_room)
            continue;
        if(!is_same_group(rch, caster))
            continue;
        if (mySpell->spellbane(caster, rch))
            continue;

        vict = wrap(rch);

        try {
            fun->invoke(thiz, funArgs);
        } catch (const CustomException &ce) {

        } catch (const VictimDeathException &vde) {
            
        }
    }

    return Register();
}

NMI_INVOKE(FeniaSpellContext, damageRoom, "(func): вызвать ф-ию для всех в комнате, кто не защищен от заклинания")
{
    Character *caster = arg2character(ch);
    RegisterList::const_iterator ai = args.begin();
    Register rfun = *ai++;
    Closure *fun = rfun.toFunction( );
    RegisterList funArgs;
    funArgs.assign(ai, args.end( ));

    for (auto &vch: caster->in_room->getPeople()) {
        if (vch->isDead())
            continue;
        if (vch->in_room != caster->in_room)
            continue;
        if (vch == caster)
            continue;
        if (is_safe_spell(caster, vch, true))
            continue;
        if (vch->is_mirror() && number_percent() < 50)
            continue;

        calcDamage();
        vict = wrap(vch);

        try {
            fun->invoke(thiz, funArgs);
        } catch (const CustomException &ce) {

        } catch (const VictimDeathException &vde) {
            
        }
    }

    return Register();
}

NMI_INVOKE(FeniaSpellContext, damageItems, "(func): вызвать ф-ию для всех предметов жертвы, предмет доступен в переменной obj")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *myVict = arg2character(vict);

    RegisterList::const_iterator ai = args.begin();
    Register rfun = *ai++;
    Closure *fun = rfun.toFunction( );
    RegisterList funArgs;
    funArgs.assign(ai, args.end( ));

    list<::Object *> items;
    for (::Object *item = myVict->carrying; item != 0; item = item->next_content)  
        items.push_back(item);

    for (auto &item: items) {
        if (item->extracted)
            continue;
        
        obj = wrap(item);

        try {
            fun->invoke(thiz, funArgs);
        } catch (const CustomException &ce) {

        } catch (const VictimDeathException &vde) {

        }        
    }

    return Register();
}

static bitstring_t my_damage_flags(const Register &ch, const Register &spell)
{
    Character *myCh = arg2character(ch);
    DefaultSpell *mySpell = arg2spell(spell);
    return mySpell->damflags | (mySpell->isPrayer(myCh) ? DAMF_PRAYER : DAMF_MAGIC);
}

static void call_effect_func(effect_fun_t &fun, FeniaSpellContext *thiz)
{
    bitstring_t damflags = my_damage_flags(thiz->ch, thiz->spell);
    void *vo = 0;
    int target;
    int level = thiz->level;
    int dam = thiz->dam;

    if (thiz->vict.type != Register::NONE) {        
        vo = arg2character(thiz->vict);
        target = TARGET_CHAR;
        
    } else if (thiz->room.type != Register::NONE) {
        vo = arg2room(thiz->room);
        target = TARGET_ROOM;
        
    } else if (thiz->obj.type != Register::NONE) {
        vo = arg2item(thiz->obj);
        target = TARGET_OBJ;
    } else
        return;

    fun(vo, level, dam, target, damflags);
}

NMI_INVOKE(FeniaSpellContext, effectCold, "(): применить холодный эффект на жертву, предмет или комнату")
{
    call_effect_func(cold_effect, this);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectFire, "(): применить огненный эффект на жертву, предмет или комнату")
{
    call_effect_func(fire_effect, this);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectSand, "(): применить эффект песчаной бури на жертву, предмет или комнату")
{
    call_effect_func(sand_effect, this);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectAcid, "(): применить кислотный эффект на жертву, предмет или комнату")
{
    call_effect_func(acid_effect, this);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectPoison, "(): применить эффект яда на жертву, предмет или комнату")
{
    call_effect_func(poison_effect, this);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectShock, "(): применить шоковый эффект на жертву, предмет или комнату")
{
    call_effect_func(shock_effect, this);
    return Register();    
}

NMI_INVOKE(FeniaSpellContext, effectScream, "(): применить эффект песчаной бури на жертву, предмет или комнату")
{
    call_effect_func(scream_effect, this);
    return Register();    
}


NMI_INVOKE(FeniaSpellContext, effectBlind, "(): применить на жертву заклинание слепоты")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);

    if (myCh->in_room == myVict->in_room && !IS_AFFECTED(myVict, AFF_BLIND))
        ::spell(gsn_blindness, level, myCh,  myVict);

    return Register();
}

NMI_INVOKE(FeniaSpellContext, effectCurse, "(): применить на жертву заклинание проклятия")
{
    if (vict.type == Register::NONE)
        return Register();

    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);

    if (myCh->in_room == myVict->in_room && !IS_AFFECTED(myVict, AFF_CURSE))
        ::spell(gsn_curse, level, myCh,  myVict);

    return Register();
}

NMI_GET(FeniaSpellContext, skill, "прототип умения для этого заклинания (.Skill())")
{
    return Register::handler<SkillWrapper>(name);    
}

NMI_SET(FeniaSpellContext, vict, "персонаж, цель заклинания для runVict - как синоним victim")
{
    Character *v = arg2character(arg);
    vict = FeniaManager::wrapperManager->getWrapper(v); // Huh?
}

NMI_SET(FeniaSpellContext, victim, "персонаж, цель заклинания для runVict - как синоним vict")
{
    return nmiSet<nmi::vict>(arg);
}

NMI_GET(FeniaSpellContext, sect, "названия типа местности в комнате кастера - как синоним sector")
{
    Character *myCh = arg2character(ch);
    return Register(sector_table.name(myCh->in_room->getSectorType()));
}

NMI_GET(FeniaSpellContext, sector, "названия типа местности в комнате кастера - как синоним sect")
{
    return nmiGet<nmi::sect>();
}

NMI_INVOKE(FeniaSpellContext, yellPanic, "(): новая жертва заклинания кричит 'помогите'")
{
    Character *myCh = arg2character(ch);
    Character *myVict = arg2character(vict);
     
    if (myCh->fighting != myVict && myVict->fighting != myCh)
        yell_panic(myCh, myVict);

    return Register();
}

static PCharacter *get_grave_owner(::Object *grave)
{
    PCharacter *victim = PCharacterManager::findPlayer(grave->getOwner());
    if (!victim || !DIGGED(victim)) {
        LogStream::sendError( ) << "Nonexisting grave owner: " << grave->getOwner() << " for obj " << grave->getID() << endl;
        return 0;
    }

    return victim;
}

static ::Object * find_grave_here(Character *ch)
{
    ::Object *grave = get_obj_room_vnum(ch->in_room, OBJ_VNUM_GRAVE);    
    if (!grave)
        return 0;

    PCharacter *victim = get_grave_owner(grave);
    if (!victim)
        return 0;
        
    if (is_safe_nomessage(ch, victim)) 
        return 0;

    return grave;
}


NMI_INVOKE(FeniaSpellContext, graveDestroy, "(): выкопать вампира из могилы в комнате кастера")
{
    Character *myCh = arg2character(ch);
    ::Object *grave = find_grave_here(myCh);
    if (!grave)
        return Register(false);

    PCharacter *victim = get_grave_owner(grave);
    if (!victim)
        return Register(false);

    undig(victim);
    return Register(true);
}

NMI_INVOKE(FeniaSpellContext, graveFind, "(): найти в комнате кастера могилу вампира в ПК-диапазоне")
{
    Character *myCh = arg2character(ch);
    ::Object *grave = find_grave_here(myCh);
    return grave ? wrap(grave) : Register();
}

NMI_INVOKE(FeniaSpellContext, graveOwner, "(): вернуть владельца могилы в комнате кастера")
{
    Character *myCh = arg2character(ch);
    ::Object *grave = find_grave_here(myCh);
    PCharacter *victim = grave ? get_grave_owner(grave) : 0;
    return victim ? wrap(victim) : Register();
}

NMI_INVOKE(FeniaSpellContext, isWater, "(): находится ли кастер в воде или под водой")
{
    Character *myCh = arg2character(ch);
    return RoomUtils::isWater(myCh->in_room);
}

NMI_INVOKE(FeniaSpellContext, hasWaterParticles, "(): достаточно ли водяных паров в комнате кастера")
{
    Character *myCh = arg2character(ch);
    return RoomUtils::hasWaterParticles(myCh->in_room);
}

NMI_INVOKE(FeniaSpellContext, isNature, "(): находится ли кастер в дикой местности")
{
    Character *myCh = arg2character(ch);
    return RoomUtils::isNature(myCh->in_room);
}

NMI_INVOKE(FeniaSpellContext, isOutside, "(): находится ли кастер снаружи помещения")
{
    Character *myCh = arg2character(ch);
    return RoomUtils::isOutside(myCh->in_room);
}

NMI_INVOKE(FeniaSpellContext, hasDust, "(): достаточно ли пыли или песка в комнате кастера")
{
    Character *myCh = arg2character(ch);
    return RoomUtils::hasDust(myCh->in_room);
}

NMI_INVOKE(FeniaSpellContext, hasParticles, "(): достаточно ли разных частиц в комнате кастера")
{
    Character *myCh = arg2character(ch);
    return RoomUtils::hasParticles(myCh->in_room);
}


NMI_GET(FeniaSpellContext, rel, "религия кастера, случайный бог для неопределившихся или строка 'бог|и|ов...' для мобов")
{
    static const char *gods = "бог|и|ов|ам|ов|ами|ах";
    Character *caster = arg2character(ch);
    
    if (caster->is_npc())
        return gods;

    if (caster->getReligion() == god_none) {
        XMLStringAttribute::Pointer randomGodAttr = caster->getPC()->getAttributes().findAttr<XMLStringAttribute>("randomGod");
        if (randomGodAttr && !randomGodAttr->getValue().empty())
            return religionManager->find(randomGodAttr->getValue())->getRussianName();
        else
            return gods;
    }
    
    return caster->getReligion()->getRussianName();
}

NMI_INVOKE(FeniaSpellContext, wait, "(seconds): пауза на указанное кол-во секунд")
{
    int delay = args2number(args) * dreamland->getPulsePerSecond();

    for (int i = 0; i < delay; i++)
        SchedulerWrapper::yield(DLString::emptyString);
    
    return Register();
}

NMI_INVOKE(FeniaSpellContext, waitSameRoom, "(seconds): пауза на указанное кол-во секунд или пока кастер в той же комнате")
{
    int delay = args2number(args) * dreamland->getPulsePerSecond();
    Character *caster = arg2character(ch);
    Room *start_room = caster->in_room;

    for (int i = 0; i < delay; i++) {
        SchedulerWrapper::yield(DLString::emptyString);
        if (caster->in_room != start_room)
            throw Scripting::CustomException("Caster left the room");
    }
    
    return Register();
}



NMI_GET(FeniaCommandContext, skill, "прототип умения для этой команды (.Skill())")
{
    return Register::handler<SkillWrapper>(name);
}

NMI_INVOKE(FeniaCommandContext, cooldown, "(duration): наложить пост-аффект на выполняющего команду, указанной длительности")
{
    Character *myCh = arg2character(ch);
    Skill *skill = arg2skill(name);
    int duration = args2number(args);
    postaffect_to_char(myCh, skill->getIndex(), duration);
    return Register();
}

NMI_INVOKE(FeniaCommandContext, takeMana, "(value): уменьшить ману персонажа на value; вернет false если не получилось")
{
    Character *myCh = arg2character(ch);
    int mana = args2number(args);

    if (myCh->mana < mana)
        return false;

    myCh->mana -= mana;
    return true;
}
