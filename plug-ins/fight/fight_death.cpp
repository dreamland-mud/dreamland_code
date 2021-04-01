/* $Id: fight_death.cpp,v 1.1.2.17 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko            {NoFate, Demogorgon}                           *
 *    Koval Nazar            {Nazar, Redrum}                                    *
 *    Doropey Vladimir            {Reorx}                                           *
 *    Kulgeyko Denis            {Burzum}                                           *
 *    Andreyanov Aleksandr  {Manwe}                                           *
 *    и все остальные, кто советовал и играл в этот MUD                           *
 ***************************************************************************/

#include <jsoncpp/json/json.h>
#include "so.h"
#include "plugin.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "fenia_utils.h"

#include "behavior_utils.h"
#include "schedulertask.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "religion.h"
#include "playerattributes.h"
#include "room.h"

#include "configurable.h"
#include "dreamland.h"
#include "save.h"
#include "interp.h"
#include "wiznet.h"
#include "messengers.h"
#include "mercdb.h"
#include "desire.h"
#include "act.h"
#include "../anatolia/handler.h"
#include "merc.h"
#include "damage.h"
#include "fight.h"
#include "vnum.h"
#include "def.h"

// A set of PK looting rules defined in fight/loot.json.
Json::Value loot;
CONFIGURABLE_LOADED(fight, loot)
{
    loot = value;
}

enum {
    LOOT_DESTROY,
    LOOT_DROP,
    LOOT_KEEP,
    LOOT_WEAR,
};

enum {
    FLOOT_PURGE = (A),
};

#define GHOST_MIN_LEVEL (PK_MIN_LEVEL + 10)

RELIG(shamash);

/*-----------------------------------------------------------------------------------------*
 * death autocommands 
 *-----------------------------------------------------------------------------------------*/
DESIRE(bloodlust);

class DeathAutoCommands {
public:
    DeathAutoCommands( Character *killer, Character *ch )
                 : killer( killer ), ch( ch ), corpse( 0 )
    {
    }

    void run( )
    {
        if (!applicable( ))
            return;
        
        findCorpse( );

        autoLook( );
        autoLoot( );
        autoGold( );
        autoBlood( );
        autoSacrifice( );
    }

protected:
    void findCorpse( )
    {
        for (Object *obj = killer->in_room->contents; obj; obj = obj->next_content)
            if (obj->item_type == ITEM_CORPSE_NPC 
                && obj->value3() == ch->getNPC( )->pIndexData->vnum) 
            {
                corpse = obj;
                break;
            }
    }

    virtual bool applicable( )
    {
        if (!killer || killer->is_npc( ))
            return false;

        if (!ch->is_npc( ))
            return false;

        if (killer->in_room != ch->in_room)
            return false;
        
        return true;
    }

    void autoLoot( )
    {
        if (!corpse || !corpse->contains) 
            return;

        if (IS_SET(killer->act, PLR_AUTOLOOT)){
            if (!killer->can_see( corpse )) {
                if (IS_AFFECTED(killer, AFF_BLIND))
                    killer->pecho("{WТы пытаешься вслепую ограбить труп, но ничего не выходит.{x");
                else
                    killer->pecho("{WТы пытаешься ограбить труп, но почему-то не находишь его.{x");
                return;
            }
            else {
                killer->pecho("{WТы методично обдираешь все вещи с трупа:{x");
                do_get_all_raw( killer, corpse );
            }
        }
    }

    void autoLook( )
    {
        if (!corpse || !corpse->contains)
            return;

        if (IS_SET(killer->add_comm, PLR_AUTOLOOK)) {            
            if (!killer->can_see( corpse )) {
                if (IS_AFFECTED(killer, AFF_BLIND))
                    killer->pecho("{WТебе хочется осмотреть труп, но ты ничего не видишь!{x");
                else
                    killer->pecho("{WТы пытаешься осмотреть труп, но почему-то не находишь его.{x");
                return;
            }
            else
                oprog_examine( corpse, killer );    
        }
    }

    void autoGold( )
    {
        if (!corpse || !corpse->contains)
            return;

        if (IS_SET(killer->act, PLR_AUTOGOLD)) {
            Object *money = get_obj_list_type( killer, ITEM_MONEY, corpse->contains );

            if (money)
                do_get_raw( killer, money, corpse );
        }
    }

    void autoBlood( )
    {
        if (desire_bloodlust->applicable( killer->getPC( ) )
        && !(IS_BLOODLESS(ch)))    
        {
            oldact("{R$c1 выпивает последние капли жизни из $C2!{x", killer, 0,ch,TO_ROOM);
            oldact("{RТы выпиваешь последние капли жизни из $C2!{x", killer, 0,ch,TO_CHAR);
            desire_bloodlust->gain( killer->getPC( ), 3 );
        }
    }

    void autoSacrifice( )
    {
        if (!corpse)
            return;
        
        if (!IS_SET(killer->act, PLR_AUTOSAC))
            return;

        if (IS_SET(killer->act, PLR_AUTOLOOT) && corpse->contains)
            return;

        interpret_raw( killer, "sacrifice", 
                       get_obj_name_list( corpse, corpse->in_room->contents, killer ).c_str( ) );
    }

    Character *killer;
    Character *ch;
    Object *corpse;
};


/*-----------------------------------------------------------------------------------------*
 * death penalties
 *-----------------------------------------------------------------------------------------*/
PROF(samurai);
void delete_player( PCharacter * );

class PlayerDeleteTask : public SchedulerTask {
public:
    typedef ::Pointer<PlayerDeleteTask> Pointer;
    
    PlayerDeleteTask( PCharacter *p, Character *k, const DLString &m )
                      : pvict( p ), killer( k ), msgWiznet( m )
    {
    }
    virtual void run( )
    {
        pvict->pecho("Ты превращаешься в привидение и покидаешь этот мир.");
        oldact("$c1 УМЕ$gРЛО|Р|РЛА, и навсегда покину$gло|л|ла этот мир.\n\r", pvict,0,0,TO_ROOM);
        wiznet( 0, 0, 0, msgWiznet.c_str( ), killer, pvict );
        
        delete_player( pvict );
    }
    virtual int getPriority( ) const
    {
        return SCDP_IOWRITE - 100;
    }

private:
    PCharacter *pvict;
    Character *killer;
    DLString msgWiznet;
};

class DeathPenalties {
public:
    DeathPenalties( Character *killer, Character *ch )
             : killer( killer ), ch( ch )
    {
        if (!ch->is_npc( ))
            pch = ch->getPC( );
    }

    bool run( ) 
    {
        if (!applicable( ))
            return false;

        pch->death++;

        return penaltyDelete( ) 
               || penaltyConstitution( )
               || penaltyCharisma( );
    }

protected:
    virtual bool applicable( ) 
    {
        if (ch->is_npc( )) 
            return false;
        
        if (ch != killer
            && !(killer->is_npc( ) && !killer->master && !killer->leader)
            && !IS_SET(ch->act, PLR_WANTED))
            return false;

        return true;
    }

    bool penaltyConstitution( )
    {
        if (pch->getProfession( ) == prof_samurai)
            return false;

        if (pch->getReligion() == god_shamash)
            return false;

        if (pch->death % 5)
            return false;

        pch->perm_stat[STAT_CON]--;

        if (pch->perm_stat[STAT_CON] >= 3) {
            pch->pecho("Ты чувствуешь, как твоя жизненная сила уменьшилась после этой смерти.");
            return false;
        }
        
        pch->perm_stat[STAT_CON] = 10;

        DLScheduler::getThis( )->putTaskNOW(
            PlayerDeleteTask::Pointer( NEW, pch, killer, 
                "%C1 : %C1 удаляется из-за слабого телосложения." ) );
        return true;
    }

    bool penaltyCharisma( )
    {
        if (pch->getProfession( ) == prof_samurai) {
            if ((pch->death % 3) == 2) {
                pch->perm_stat[STAT_CHA]--;
                pch->pecho("Ты чувствуешь, как твое обаяние уменьшается после этой смерти.");
            }
        }

        return false;
    }

    bool penaltyDelete( )
    {
        if (pch->getProfession( ) != prof_samurai)
            return false;
            
        if (pch->death <= 10)
            return false;
            
        pch->death = 0;

        DLScheduler::getThis( )->putTaskNOW(
            PlayerDeleteTask::Pointer( NEW, pch, killer, 
                "%C1 : %C1 удаляется, достигнув предела 10 смертей самурая." ) );
        return true;
    }

    Character *killer;
    Character *ch;
    PCharacter *pch;
};

class DeathEventHandler : public Plugin, public EventHandler {
public:
    typedef ::Pointer<DeathEventHandler> Pointer;

    virtual void initialization()
    {
        eventBus->subscribe(typeid(CharDeathEvent), Pointer(this));
    }

    virtual void destruction()
    {
        eventBus->unsubscribe(typeid(CharDeathEvent), Pointer(this));
    }

    virtual void handleEvent(const type_index &eventType, const Event &event) const
    {
        const CharDeathEvent &evt = static_cast<const CharDeathEvent &>(event);
        Character *victim = evt.victim;
        Character *killer = evt.killer;

        //transfer the killer flag to master if killer is a charmed npc
        if (killer->is_npc() && killer->master && !killer->master->is_npc())
            killer = killer->master;

        DeathPenalties( killer, victim ).run( );

        group_gain( killer, victim, evt.killer );
        raw_kill( victim, -1, killer, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
        pk_gain( killer, victim );
    }
};

static void loot_transform( Object *obj, Character *ch )
{
    // ROT_DEATH flag is only effective when the second owner dies (i.e. not the original mob that was reset with this item). 
    // Keys are the only historic exception to this rule and begin to rot immediately after mob's death.
    if (IS_SET(obj->extra_flags, ITEM_ROT_DEATH)) {
        if (ch->is_npc() && obj->reset_mob == ch->getID() && obj->item_type != ITEM_KEY) {
            // Do nothing, this was the original owner.
        } else {
            // Sic transit
            obj->timer = number_range( 5, 10 );
            REMOVE_BIT(obj->extra_flags, ITEM_ROT_DEATH);
        }
    }

    REMOVE_BIT(obj->extra_flags, ITEM_VIS_DEATH);

    switch (obj->item_type) {
    case ITEM_POTION:
        obj->timer = number_range( 500, 1000 );
        break;
    case ITEM_SCROLL:
        obj->timer = number_range( 1000, 2500 );
        break;
    case ITEM_WEAPON:
        if (ch->is_npc( ))
            obj->condition = min(obj->condition, number_range( 30, 80 ));
        break;
    case ITEM_ARMOR:
        if (ch->is_npc( ))
            obj->condition = min(obj->condition, number_range( 10, 70 ));
        break;
    case ITEM_FOOD:
        if (ch->is_npc( ))
            obj->condition = min(obj->condition, number_range( 10, 50 ));
        break;
    }
}

static int loot_position( Object *obj, Character *ch, int flags = 0 )
{
    if (obj->wear_loc == wear_tattoo) 
        return LOOT_WEAR;

    if (IS_SET( obj->extra_flags, ITEM_INVENTORY )) 
        return LOOT_DESTROY;
   
    if (obj->pIndexData->limit != -1 && obj->pIndexData->count > obj->pIndexData->limit) 
        return LOOT_DESTROY;
    
    if (IS_SET(flags, FLOOT_PURGE)) {
        if (IS_SET(obj->extra_flags, ITEM_NOPURGE) 
            || IS_SET(obj->pIndexData->extra_flags, ITEM_NOPURGE))
            return LOOT_DROP;

        return LOOT_DESTROY;
    }
    else {
        if (!ch->is_npc( ) && ch->getPC( )->getAttributes( ).isAvailable( "fullloot" ))
            return LOOT_DROP;
        
        return LOOT_KEEP;
    }
}

static Object * corpse_create( Character *ch )
{
    Object *corpse;
    DLString name;

    if (ch->is_npc( ) && IS_SET( ch->form, FORM_INSTANT_DECAY|FORM_INTANGIBLE )) {
        ch->in_room->echo( POS_RESTING, "%1$^C1 превращается в прах и развеивается по ветру, не оставляя после себя трупа.", ch );
        return NULL;
    }    
    
    name = ch->getNameP( '2' );
    corpse = create_object(
                    get_obj_index(
                        ch->is_npc( ) ? OBJ_VNUM_CORPSE_NPC : OBJ_VNUM_CORPSE_PC), 0);

    if (ch->is_npc( )) {
        corpse->timer = number_range( 3, 6 );
    }
    else {
        corpse->timer = number_range( 20, 25 );
        corpse->setOwner( ch->getNameP( ) );
    }

    corpse->from = str_dup( name.c_str( ) );
    corpse->cost = 0;
    corpse->level = ch->getRealLevel( );
    corpse->fmtShortDescr( corpse->getShortDescr( ), name.c_str( ) ); 
    corpse->fmtDescription( corpse->getDescription( ), name.c_str( ) ); 

    if (IS_SET(ch->form, FORM_EDIBLE))
        corpse->value0((1 << ch->size) - 1);

    corpse->value1(ch->getModifyLevel( ));
    corpse->value2(ch->parts);

    if (ch->is_npc( ))
        corpse->value3(ch->getNPC( )->pIndexData->vnum);
    else
        corpse->value3(ch->getPC( )->getHometown( )->getPit( ));
    
    corpse->value4(ch->alignment);
    return corpse;
}

static void corpse_looting( Object *corpse, Character *ch, Character *killer )
{
    if (!corpse)
        return;

    if (!killer)
        return;
        
    if (ch->is_npc( ))
        return;

    if (killer->is_npc())
        return;

    list<Object *> items = corpse->getItems();

    if (IS_SET(ch->act, PLR_WANTED)) {
        corpse->killer = str_dup( "!anybody!" );
        corpse->count = items.size() * loot["wantedCountCoeff"].asFloat();
    }
    else {
        corpse->killer = str_dup( killer->getNameP( ) );
        corpse->count = loot["maxTotalCount"].asInt();
    }

    // Mark items with a "looting" property based on the rules. 
    // TODO: modify 'get' logic to forbid looting items with "loot"="false".
    // TODO: modify show_list_to_char to highlight items one can loot.
    const DLString mark("loot");

    for (auto &obj: items) {
        obj->removeProperty(mark);

        if (obj->item_type == ITEM_MONEY)
            obj->addProperty(mark, loot["money"].asString());
        else if (obj->pIndexData->limit > 0)
            obj->addProperty(mark, loot["limits"].asString());
        else if (!obj->getProperty("tier").empty())
            obj->addProperty(mark, loot["randoms"].asString());
        // TODO: add more checks for different types of items.
        else
            obj->addProperty(mark, loot["other"].asString());
    }
}

static void corpse_money( Object *corpse, Character *ch )
{
    Object *money;
    
    if (ch->gold <= 0 && ch->silver <= 0)
        return;
    
    money = create_money( ch->gold, ch->silver );

    if (corpse)
        obj_to_obj( money, corpse );
    else
        obj_to_room( money, ch->in_room );

    ch->gold = 0;
    ch->silver = 0;
}

static void corpse_place( Object *corpse, Character *ch )
{
    Room *corpse_room = 0;
    
    if (!corpse)
        return;

    if (!ch->is_npc( ) && ch->getModifyLevel( ) < GHOST_MIN_LEVEL) 
        corpse_room = get_room_instance( ch->getPC()->getHometown( )->getAltar( ) );
    
    if (!corpse_room)
        corpse_room = ch->in_room;

    obj_to_room( corpse, corpse_room );
}

static void corpse_fill( Object *corpse, Character *ch, int flags = 0 )
{
    Object *obj, *obj_next;
    Wearlocation *wearloc;
    DLString worn;

    for (obj = ch->carrying; obj; obj = obj_next) {
        obj_next = obj->next_content;
        
        switch (loot_position( obj, ch, flags )) {
        case LOOT_DESTROY:
            extract_obj( obj );
            continue;

        case LOOT_DROP:
            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
            break;

        case LOOT_KEEP:
            // Remember where the item have been worn to correctly format 'get' command messages.
            worn = obj->wear_loc != wear_none ? 
                        obj->wear_loc->getName() : DLString::emptyString;
            obj_from_char( obj );

            if (corpse) {
                free_string(obj->from);
                obj->from = str_dup(worn.c_str());
                obj_to_obj( obj, corpse );
            } else 
                obj_to_room( obj, ch->in_room );
            break;

        case LOOT_WEAR:
            wearloc = &*obj->wear_loc;
            wearloc->unequip( obj );
            obj->wear_loc.assign( wearloc );
            break;
        }

        loot_transform( obj, ch );
    }
}    

static void corpse_reequip( Character *victim )
{
    for (Object *obj = victim->carrying; obj; obj = obj->next_content)
        obj->wear_loc->equip( obj );
}

/*  
 *  disintegrate the objects... 
 */
void purge_corpse( Character *killer, Character *ch )
{
    dreamland->removeOption( DL_SAVE_OBJS );
    dreamland->removeOption( DL_SAVE_MOBS );

    corpse_fill( NULL, ch, FLOOT_PURGE );
    ch->gold = 0;
    ch->silver = 0;

    dreamland->resetOption( DL_SAVE_OBJS );
    dreamland->resetOption( DL_SAVE_MOBS );

    save_items( ch->in_room );
    save_mobs( ch->in_room );
}

/*
 * Make a corpse out of a character.
 */
void make_corpse( Character *killer, Character *ch )
{
    Object *corpse;
    
    if (ch->is_mirror( ))
        return;
    
    dreamland->removeOption( DL_SAVE_OBJS );
    dreamland->removeOption( DL_SAVE_MOBS );

    corpse = corpse_create( ch );
    corpse_money( corpse, ch );
    corpse_place( corpse, ch );
    corpse_fill( corpse, ch );
    corpse_looting( corpse, ch, killer );
    ch->getClan( )->makeMonument( ch, killer );

    dreamland->resetOption( DL_SAVE_OBJS );
    dreamland->resetOption( DL_SAVE_MOBS );

    save_items( ch->in_room );
    save_mobs( ch->in_room );

    if (corpse && corpse->in_room != ch->in_room)
        save_items( corpse->in_room );
}

Object * bodypart_create( int vnum, Character *ch, Object *corpse )
{
    Object *obj;
    DLString body_name;
    bitstring_t body_form = 0;
    int body_level = 0, body_vnum = 0;
    Room *body_room;
    
    if (ch) {
        body_name = ch->getNameP( '2' );
        body_form = ch->form;
        body_level = ch->getModifyLevel( );
        body_room = ch->in_room;
        body_vnum = ch->is_npc( ) ? ch->getNPC( )->pIndexData->vnum : 0;
    }
    else if (corpse) {
        body_name = corpse->getShortDescr( '1' ).replaces( "труп ", "" );
        if (body_name.size( ) == corpse->getShortDescr( '1' ).size( ))
            body_name = "";
        
        if (corpse->value3()) {
            MOB_INDEX_DATA *pBodyOwner;
            if (( pBodyOwner = get_mob_index( corpse->value3() ) ))
                body_form = pBodyOwner->form;
        }

        body_level = corpse->value1();
        body_room = corpse->getRoom( );
        body_vnum = corpse->value3();
    }
    else {
        body_room = get_room_instance( ROOM_VNUM_LIMBO );
    }

    obj        = create_object( get_obj_index( vnum ), 0 );
    obj->timer = number_range( 4, 7 );
    
    obj->fmtShortDescr( obj->getShortDescr( ), body_name.c_str( ) );
    obj->fmtDescription( obj->getDescription( ), body_name.c_str( ) );
    obj->from = str_dup(body_name.c_str());
    obj->level = body_level;

    if (obj->item_type == ITEM_FOOD) {
        if (IS_SET(body_form, FORM_POISON))
            obj->value3(1);
        else if (!IS_SET(body_form, FORM_EDIBLE))
            obj->item_type = ITEM_TRASH;
    }
    
    obj->value4(body_vnum);
    obj_to_room( obj, body_room );

    return obj;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( Character *ch, int part )
{
    const char *msg;
    int vnum;
    Bitstring bodyparts( ch->parts );

    if (ch->is_mirror( ))
        return;

    vnum = 0;
    msg = "Ты слышишь предсмертный крик $c2.";

    if (part == -1)
      part = number_bits(4);

    switch (part)
    {
    case  0: msg  = "$c1 падает на землю... и {RУМИРАЕТ{x.";            
             break;
    case  1:
        if (!IS_BLOODLESS(ch))
        {
            msg  = "Кровь $c2 покрывает твои доспехи.";        
            break;
        }
    case  2:                             
        if (bodyparts.isSet(PART_GUTS))
        {
            msg = "Внутренности $c2 вываливаются тебе под ноги.";
            vnum = OBJ_VNUM_GUTS;
            bodyparts.removeBit(PART_GUTS);
        }
        break;
    case  3:
        if (bodyparts.isSet(PART_HEAD))
        {
            msg  = "Отрубленная голова $c2 падает тебе под ноги.";
            vnum = OBJ_VNUM_SEVERED_HEAD;                
            bodyparts.removeBit(PART_HEAD);
        }
        break;
    case  4:
        if (bodyparts.isSet(PART_HEART))
        {
            msg  = "Сердце $c2 выпадает из $s разрубленной груди.";
            vnum = OBJ_VNUM_TORN_HEART;                
            bodyparts.removeBit(PART_HEART);
        }
        break;
    case  5:
        if (bodyparts.isSet(PART_ARMS))
        {
            msg  = "Рука $c2 отваливается от $s мертвого тела.";
            vnum = OBJ_VNUM_SLICED_ARM;                
            bodyparts.removeBit(PART_ARMS);
        }
        break;
    case  6:
        if (bodyparts.isSet(PART_LEGS))
        {
            msg  = "Нога $c2 отваливается от $s мертвого тела.";
            vnum = OBJ_VNUM_SLICED_LEG;                
            bodyparts.removeBit(PART_LEGS);
        }
        break;
    case 7:
        if (bodyparts.isSet(PART_BRAINS))
        {
            msg = "Голова $c2 разлетается как спелый арбуз, забрызгивая все вокруг.";
            vnum = OBJ_VNUM_BRAINS;
            bodyparts.removeBit(PART_BRAINS);
        }
    }

    oldact( msg, ch, 0, 0, TO_ROOM );

    if (vnum != 0)
        bodypart_create( vnum, ch, NULL ); 

    msg = "Ты слышишь чей-то предсмертный крик.";
    ch->in_room->echoAround( POS_RESTING, msg );
    ch->parts = bodyparts;
}

void reset_dead_player( PCharacter *victim )
{
    for (auto &paf: victim->affected.clone())
        affect_remove( victim, paf );

    victim->affected_by    = 0;
    victim->add_affected_by = 0;
    victim->detection    = 0;
    victim->armor.clear( );
    victim->armor.fill( 100 );
    victim->position    = POS_STANDING;
    victim->hit        = victim->max_hit / 10;
    victim->mana    = victim->max_mana / 10;
    victim->move    = victim->max_move;
    victim->shadow = -1;

    for (int i = 0; i < desireManager->size( ); i++)
        desireManager->find( i )->reset( victim );
}

void killed_npc_gain( NPCharacter *victim )
{
    victim->getNPC( )->pIndexData->killed++;
    kill_table[URANGE(0, victim->getRealLevel( ), MAX_LEVEL-1)].killed++;
}

void ghost_gain( Character *victim )
{
    if (!victim->is_npc( )) {
        SET_DEATH_TIME(victim);

        if (victim->getModifyLevel( ) >= GHOST_MIN_LEVEL) {
            victim->pecho("Ты лишаешься тела на несколько минут.");
            set_ghost( victim );
        }
    }
}

static bool oprog_death( Character *victim, Character *killer )
{
    Object *obj, *obj_next;
    
    for (obj = victim->carrying; obj != 0; obj = obj_next) {
        obj_next = obj->next_content;    
        
        FENIA_CALL( obj, "Death", "CC", victim, killer )
        FENIA_NDX_CALL( obj, "Death", "OCC", obj, victim, killer )
        BEHAVIOR_CALL( obj, death, victim )
    }

    return false;
}

void loyalty_gain( Character *ch, Character *victim, int flags )
{
    if (!ch)
        return;

    if (victim->is_npc( ))
        return;
        
    if (!IS_SET(victim->act, PLR_WANTED))
        return;
    
    if (!ch->is_npc( ))
        ch->getPC( )->loyalty = min ( ch->getPC( )->loyalty+25, 1000 );
    
    if (IS_SET(flags, FKILL_REABILITATE) || victim->getPC( )->loyalty > -900)
        REMOVE_BIT(victim->act, PLR_WANTED);
}

static bool mprog_death( Character *victim, Character *killer )
{
    FENIA_CALL( victim, "Death", "C", killer );
    FENIA_NDX_CALL( victim->getNPC( ), "Death", "CC", victim, killer );
    BEHAVIOR_CALL( victim->getNPC( ), death, killer );
    return false;
}

static bool mprog_kill( Character *killer, Character *victim )
{
    FENIA_CALL( killer, "Kill", "C", victim );
    FENIA_NDX_CALL( killer->getNPC( ), "Kill", "CC", killer, victim );
    BEHAVIOR_CALL( killer->getNPC( ), kill, victim );
    return false;
}

void raw_kill( Character* victim, int part, Character* ch, int flags )
{
    int wizflag;

    stop_fighting( victim, true );
    
    if (oprog_death( victim, ch )) {
        victim->position = POS_STANDING;
        return;
    }
    
    victim->unsetLastFightTime( );

    gprog("onDeath", "CC", victim, ch);

    if (mprog_death( victim, ch )) {
        victim->setDead( );
        return;
    }
    
    if (IS_SET(flags, FKILL_CRY))
        death_cry( victim, part );

    if (IS_SET(flags, FKILL_CORPSE))
        make_corpse( ch, victim );
    else if (IS_SET(flags, FKILL_PURGE))
        purge_corpse( ch, victim );
    
    static const char *msg_killed = "%1$C1 па%1$Gло|л|ла от руки %2$C2.";
    static const char *msg_died = "%1$C1 погиб%1$Gло||ла своей смертью.";
    const char *&msg = (ch && victim != ch) ? msg_killed : msg_died;

    wizflag = (victim->is_npc( ) ? WIZ_MOBDEATHS : WIZ_DEATHS);
    wiznet(wizflag, 0, victim->get_trust(), msg, victim, ch);

    if (!victim->is_npc()) {
        send_discord_death(fmt(0, msg, victim, ch));
        if (ch && !ch->is_npc() && ch != victim)
            send_telegram(fmt(0, msg, victim, ch));
    }

    if (ch && mprog_kill( ch, victim ))
        ch = NULL;

    if (victim->is_npc( )) {
        killed_npc_gain( victim->getNPC( ) );

        if (IS_SET(flags, FKILL_MOB_EXTRACT))
            extract_char( victim );
        else
        {
            victim->setDead( );
            DeathAutoCommands( ch, victim ).run( );   
        }    

        return;
    }

    victim->getPC( )->getAttributes( ).handleEvent( DeathArguments( victim->getPC( ), ch ) );
    
    extract_dead_player( victim->getPC( ), FEXTRACT_COUNT | (IS_SET(flags, FKILL_PURGE) ? FEXTRACT_LASTFOUGHT : 0));
    reset_dead_player( victim->getPC( ) );
    loyalty_gain( ch, victim, flags );

    if (IS_SET(flags, FKILL_GHOST))
        ghost_gain( victim );
    
    corpse_reequip( victim );

    victim->getPC( )->save( );
}

void pk_gain( Character *killer, Character *victim )
{
    if ( !killer->is_npc() && !victim->is_npc() && killer != victim )
    {
        set_killer( killer );
        set_slain( victim );
        killer->getClan( )->handleVictory( killer->getPC( ), victim->getPC( ) );
        victim->getClan( )->handleDefeat( victim->getPC( ), killer->getPC( ) );

    }
}

extern "C"
{
    SO::PluginList initialize_fight( )
    {
        SO::PluginList ppl;

        Plugin::registerPlugin<DeathEventHandler>( ppl );
        return ppl;
    }
}

