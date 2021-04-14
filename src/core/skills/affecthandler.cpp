/* $Id$
 *
 * ruffina, 2004
 */
#include "affecthandler.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "grammar_entities_impl.h"
#include "character.h"
#include "object.h"
#include "room.h"

AffectHandler::~AffectHandler( )
{
}


void AffectHandler::remove( Character *ch )
{
}

void AffectHandler::remove( Object *obj )
{
}

void AffectHandler::remove( Room * room )
{
}

void AffectHandler::update( Character * ch, Affect * )
{
}

void AffectHandler::update( Object *obj, Affect * )
{
}

void AffectHandler::update( Room *room, Affect * )
{
}

void AffectHandler::entry( Character *, Affect * )
{
}

void AffectHandler::entry( Room *room, Character *, Affect * )
{
}

void AffectHandler::leave( Room *room, Character *, Affect * )
{
}

void AffectHandler::look( Character *, Character *, Affect * )
{
}

bool AffectHandler::smell( Character *, Character *, Affect * )
{
    return false;
}

void AffectHandler::dispel( Character *ch )
{
}

void AffectHandler::toStream( ostringstream &, Affect * ) 
{
}

void AffectHandler::saves( Character *, Character *, int &, int, Affect * )
{
}

void AffectHandler::stopfol( Character *, Affect * )
{
}

bool AffectHandler::isDispelled( ) const
{
    return false;
}

bool AffectHandler::isCancelled( ) const
{
    return false;
}

bool AffectHandler::onFight(const SpellTarget::Pointer &target, Affect *paf, Character *victim)
{
    AffectHandler *ah = this;
    switch (target->type) {
        case SpellTarget::CHAR:
            FENIA_CALL(ah, "FightChar", "CAC", target->victim, paf, victim);
            break;

        default:
            break;
    }

    return false;
}

bool AffectHandler::onImmune(const SpellTarget::Pointer &target, Affect *paf, Character *attacker, int &dam, const char *damType, Object *wield, int damFlag, const char *skillName) 
{
    AffectHandler *ah = this;
    switch (target->type) {
        case SpellTarget::CHAR:
            FENIA_NUM_CALL(ah, "Immune", dam, "CACisOis", target->victim, paf, attacker, dam, damType, wield, damFlag, skillName);
            break;

        default:
            break;
    }

    return false;
}

bool AffectHandler::onHit(const SpellTarget::Pointer &target, Affect *paf, Character *attacker, int dam, const char *damType, Object *wield) 
{
    AffectHandler *ah = this;
    switch (target->type) {
        case SpellTarget::CHAR:
            FENIA_CALL(ah, "Hit", "CACisO", target->victim, paf, attacker, dam, damType, wield);
            break;

        default:
            break;
    }

    return false;
}

bool AffectHandler::onRemove(const SpellTarget::Pointer &target, Affect *paf) 
{
    AffectHandler *ah = this;
    switch (target->type) {
        case SpellTarget::ROOM:
            FENIA_CALL(ah, "RemoveRoom", "RA", target->room, paf);
            remove(target->room);
            break;

        case SpellTarget::CHAR:
            FENIA_CALL(ah, "RemoveChar", "CA", target->victim, paf);
            remove(target->victim);
            break;

        case SpellTarget::OBJECT:
            FENIA_CALL(ah, "RemoveObj", "OA", target->obj, paf);
            remove(target->obj);
            break;        

        default:
            return false;
    }

    return false;    
}

bool AffectHandler::onGet(const SpellTarget::Pointer &target, Affect *paf, Character *actor) 
{
    AffectHandler *ah = this;

    if (target->type != SpellTarget::OBJECT)
        return false;

    FENIA_CALL(ah, "Get", "OAC", target->obj, paf, actor);
    return false;
}

bool AffectHandler::onSpec(const SpellTarget::Pointer &target, Affect *paf) 
{
    AffectHandler *ah = this;
    switch (target->type) {
        case SpellTarget::ROOM:
            FENIA_CALL(ah, "SpecRoom", "RA", target->room, paf);
            break;

        case SpellTarget::CHAR:
            FENIA_CALL(ah, "SpecChar", "CA", target->victim, paf);
            break;

        case SpellTarget::OBJECT:
            FENIA_CALL(ah, "SpecObj", "OA", target->obj, paf);
            break;       

        default:
            return false; 
    }

    return false;    
}

bool AffectHandler::onPourOut(const SpellTarget::Pointer &target, Affect *paf, Character *actor, Object *out, const char *liqname, int amount) 
{
    AffectHandler *ah = this;

    if (target->type != SpellTarget::OBJECT)
        return false;

    FENIA_CALL(ah, "PourOut", "OACOsi", target->obj, paf, actor, out, liqname, amount);
    return false;    
}

bool AffectHandler::onUpdate(const SpellTarget::Pointer &target, Affect *paf) 
{
    AffectHandler *ah = this;
    switch (target->type) {
        case SpellTarget::ROOM:
            FENIA_CALL(ah, "UpdateRoom", "RA", target->room, paf);
            update(target->room, paf);
            break;

        case SpellTarget::CHAR:
            FENIA_CALL(ah, "UpdateChar", "CA", target->victim, paf);
            update(target->victim, paf);
            break;

        case SpellTarget::OBJECT:
            FENIA_CALL(ah, "UpdateObj", "OA", target->obj, paf);
            update(target->obj, paf);
            break;       

        default:
            return false; 
    }

    return false;
}

bool AffectHandler::onEntry(const SpellTarget::Pointer &target, Affect *paf, Character *walker) 
{
    Character *ch;
    Room *room;
    AffectHandler *ah = this;

    if (target->type == SpellTarget::CHAR)  {
        ch = target->victim;
        room = ch->in_room;
    } else if (target->type == SpellTarget::ROOM) {
        room = target->room;
        ch = walker;
    } else
        return false;

    FENIA_CALL(ah, "Entry", "RCA", room, ch, paf);
    
    if (target->type == SpellTarget::ROOM)
        entry(room, ch, paf);
    else
        entry(ch, paf);

    return false;
}

bool AffectHandler::onLeave(const SpellTarget::Pointer &target, Affect *paf, Character *walker) 
{
    Character *ch;
    Room *room;
    AffectHandler *ah = this;

    if (target->type == SpellTarget::ROOM) {
        room = target->room;
        ch = walker;
    } else
        return false;

    FENIA_CALL(ah, "Leave", "RCA", room, ch, paf);
    leave(room, ch, paf);
    return false;
}

bool AffectHandler::onDispel(const SpellTarget::Pointer &target, Affect *paf) 
{
    Character *ch = target->victim;
    AffectHandler *ah = this;

    FENIA_CALL( ah, "Dispel", "CA", ch, paf );
    dispel(ch);
    return false;
}

bool AffectHandler::onLook(const SpellTarget::Pointer &target, Affect *paf, Character *looker) 
{
    Character *ch = target->victim;
    AffectHandler *ah = this;

    FENIA_CALL( ah, "Look", "CAC", ch, paf, looker);
    look(looker, ch, paf);
    return false;
}

bool AffectHandler::onSmell(const SpellTarget::Pointer &target, Affect *paf, Character *sniffer) 
{    
    AffectHandler *ah = this;

    if (target->type == SpellTarget::CHAR) {
        FENIA_CALL( ah, "SmellChar", "CAC", target->victim, paf, sniffer );
        return smell(target->victim, sniffer, paf);

    } else if (target->type == SpellTarget::ROOM) {
        FENIA_CALL( ah, "SmellRoom", "RAC", target->room, paf, sniffer );

    } else if (target->type == SpellTarget::OBJECT) {
        FENIA_CALL( ah, "SmellObj", "OAC", target->obj, paf, sniffer );

    }
    
    return false;
}

bool AffectHandler::onSaves(const SpellTarget::Pointer &target, Affect *paf, Character *victim, int &save, int dam_type) 
{
    Character *ch = target->victim;
    AffectHandler *ah = this;

    FENIA_NUM_CALL(ah, "Saves", save, "CACis", ch, paf, victim, save, dam_type);
    saves(ch, victim, save, dam_type, paf);
    return false;
}

bool AffectHandler::onStopfol(const SpellTarget::Pointer &target, Affect *paf) 
{
    Character *ch = target->victim;
    AffectHandler *ah = this;

    FENIA_CALL(ah, "Stopfol", "CA", ch, paf);
    stopfol(ch, paf);
    return false;
}

static DLString afprog_descr(AffectHandler *ah, Character *ch, Affect *paf)
{
    FENIA_STR_CALL(ah, "Descr", "CA", ch, paf);
    return DLString::emptyString;
}

bool AffectHandler::onDescr(const SpellTarget::Pointer &target, Affect *paf, ostringstream &buf) 
{
    Character *ch = target->victim;
    DLString d = afprog_descr(this, ch, paf);
    if (!d.empty())
        buf << d;
    else
        toStream(buf, paf);
    return false;
}

static DLString afprog_show(AffectHandler *ah, Object *obj, Character *looker, Affect *paf)
{
    FENIA_STR_CALL(ah, "Show", "OAC", obj, paf, looker);
    return DLString::emptyString;
}

bool AffectHandler::onShow(const SpellTarget::Pointer &target, Affect *paf, Character *looker, ostringstream &buf) 
{
    AffectHandler *ah = this;

    if (target->type != SpellTarget::OBJECT)
        return false;

    DLString d = afprog_show(ah, target->obj, looker, paf);
    if (!d.empty())
        buf << d;
    return false;
}
