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

#include "core/behavior/behavior_utils.h"
#include "schedulertask.h"
#include "dlscheduler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "religion.h"
#include "playerattributes.h"
#include "player_account.h"
#include "room.h"
#include "affect.h"
#include "liquid.h"

#include "configurable.h"
#include "dreamland.h"
#include "save.h"
#include "interp.h"
#include "wiznet.h"
#include "messengers.h"
#include "infonet.h"
#include "areaquestutils.h"
#include "desire.h"
#include "act.h"
#include "loadsave.h"
#include "fight_extract.h"
#include "merc.h"
#include "damage.h"
#include "fight.h"
#include "vnum.h"
#include "def.h"

using namespace std;

// Temporary kill statistics.
player_kill_stat_t player_kill_stat;

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
GSN(poured_liquid);
LIQ(blood);

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
                interpret_raw(killer, "get", "all %lld", corpse->getID());
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
            else {
                interpret_raw(killer, "look", "in %lld", corpse->getID());
            }
        }
    }

    void autoGold( )
    {
        if (!corpse || !corpse->contains)
            return;

        if (IS_SET(killer->act, PLR_AUTOGOLD)) {
            Object *money = get_obj_list_type( killer, ITEM_MONEY, corpse->contains );

            if (money)
                interpret_raw(killer, "get", "%lld %lld", money->getID(), corpse->getID());
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

class PlayerDeleteTask : public SchedulerTask {
public:
    typedef ::Pointer<PlayerDeleteTask> Pointer;
    
    PlayerDeleteTask( PCharacter *p, Character *k, const DLString &m )
                      : pvict( p ), killer( k ), msgWiznet( m )
    {
    }
    virtual void run( )
    {
        pvict->pecho("Милостивая Варда больше не сможет тебя возродить.");
        pvict->pecho("Ты превращаешься в призрак в последний раз и навсегда покидаешь этот мир.");
        oldact("$c1 УМЕ$gРЛО|Р|РЛА, и навсегда покину$gло|л|ла этот мир.\n\r", pvict,0,0,TO_ROOM);
        
        DLString msg = fmt(0, msgWiznet.c_str(), pvict, killer);
        wiznet( 0, 0, 0, msg.c_str());
        infonet(pvict, 0, "{CТихий голос из $o2: ", msg.c_str());
        send_to_discord_stream(":ghost: " + msg);
        send_telegram(msg.c_str());

        Player::quitAndDelete( pvict );
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
        
        if (!killer)
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
            pch->pecho("Ты чувствуешь, как твое телосложение уменьшается после этой смерти.");
            return false;
        }
        
        pch->perm_stat[STAT_CON] = 10;

        DLScheduler::getThis( )->putTaskNOW(
            PlayerDeleteTask::Pointer( NEW, pch, killer, 
                "%C1 теряет слишком много очков телосложения и навсегда покидает этот мир." ) );
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
                "%C1 десятый раз гибнет смертью самурая и навсегда покидает этот мир" ) );
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
        bitstring_t flags = evt.flags;
        DLString label = evt.label;
        int damtype = evt.damtype;

        raw_kill( victim, flags, killer, label, damtype );
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
        ch->in_room->echo( POS_RESTING, "%1$^C1 превраща%1$nется|ются в прах и развеива%1$nется|ются по ветру, не оставляя после себя трупа.", ch );
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
        corpse->setOwner( ch->getNameC() );
    }

    corpse->from = str_dup( name.c_str( ) );
    corpse->cost = 0;
    corpse->level = ch->getRealLevel( );
    corpse->setShortDescr( fmt(0, corpse->getShortDescr( ), name.c_str( )) ); 
    corpse->setDescription( fmt(0, corpse->getDescription( ), name.c_str( ) )); 

    if (IS_SET(ch->form, FORM_EDIBLE))
        corpse->value0((1 << ch->size) - 1);

    corpse->value1(ch->getModifyLevel( ));
    corpse->value2(ch->parts);

    if (ch->is_npc( ))
        corpse->value3(ch->getNPC( )->pIndexData->vnum);
    else
        corpse->value3(ch->getPC( )->getHometown( )->getPit( ));
    
    corpse->value4(ch->alignment);
    switch ( ch->size ) {
        default:
        case 0:
            corpse->weight = number_range(20, 120);
            break;
        case 1:
            corpse->weight = number_range(140, 800);
            break;
        case 2:
            corpse->weight = number_range(800, 2200);
            break;
        case 3:
            corpse->weight = number_range(3000, 6000);
            break;
        case 4:
            corpse->weight = number_range(6500, 10000);
            break;
        case 5:
            corpse->weight = number_range(11000, 18000);
            break;
        case 6:
            corpse->weight = number_range(20000, 30000);
            break;
    }
    
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
        corpse->killer = str_dup( killer->getNameC() );
        corpse->count = loot["maxTotalCount"].asInt();
    }

    // Mark items with a "looting" property based on the rules. 
    const DLString mark("loot");

    for (auto &obj: items) {
        obj->removeProperty(mark);

        if (obj->item_type == ITEM_MONEY)
            obj->setProperty(mark, loot["money"].asString());
        else if (obj->pIndexData->limit > 0)
            obj->setProperty(mark, loot["limits"].asString());
        else if (!obj->getProperty("tier").empty())
            obj->setProperty(mark, loot["randoms"].asString());
        else if (obj->item_type == ITEM_PILL 
                || obj->item_type == ITEM_SCROLL
                || obj->item_type == ITEM_POTION
                || obj->item_type == ITEM_WAND
                || obj->item_type == ITEM_STAFF
                || obj->item_type == ITEM_WARP_STONE)
            obj->setProperty(mark, loot["magic"].asString());
        else if (obj->item_type == ITEM_RECIPE)
            obj->setProperty(mark, loot["recipes"].asString());
        else
            obj->setProperty(mark, loot["other"].asString());
    }
}

static void corpse_money( Object *corpse, Character *ch )
{
    Object *money;
    
    if (ch->gold <= 0 && ch->silver <= 0)
        return;
    
    money = Money::create( ch->gold, ch->silver );

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
 * Make a corpse out of a character.
 */
static void make_corpse( Character *killer, Character *ch )
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
    
    // Get body part owner name and other details. 
    // For body parts created upon 'ch' death - take details directly from character.
    // For parts chopped off from existing corpses - guess from the corpse short description.
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
    
    // Format body part name, adding owner name to its description, e.g. "отрезанная рука Керрада"
    // If there're no format symbols in the body part names, just concatenate owner name to it.
    if (str_str(obj->getShortDescr(), "%"))
        obj->setShortDescr( fmt(0, obj->getShortDescr(), body_name.c_str()));
    else
        obj->setShortDescr(obj->getShortDescr() + DLString::SPACE + body_name);

    if (str_str(obj->getDescription(), "%"))
        obj->setDescription(fmt(0, obj->getDescription(), body_name.c_str()));
    else
        obj->setDescription(obj->getDescription() + DLString::SPACE + body_name);

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

static void killed_npc_gain( Character *killer, NPCharacter *victim )
{
    victim->getNPC( )->pIndexData->killed++;

    if (killer && !killer->is_npc()) {
        int vnum = victim->getNPC()->pIndexData->vnum;
        DLString playerName = killer->getName();

        auto &kill_stat = player_kill_stat[playerName][vnum];
        kill_stat.first++; // total counter
        kill_stat.second = dreamland->getCurrentTime(); // last kill time
    }
}

static void ghost_gain( Character *victim )
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

// Killing a wanted players increases your loyalty.
static void loyalty_gain( Character *ch, Character *victim )
{
    if (!ch)
        return;

    if (!IS_SET(victim->act, PLR_WANTED))
        return;
    
    if (!ch->is_npc( ))
        ch->getPC( )->setLoyalty(min ( ch->getPC( )->getLoyalty()+25, 1000 ));
    
    if (victim->getPC( )->getLoyalty() > -900)
        REMOVE_BIT(victim->act, PLR_WANTED);
}

static bool mprog_death( Character *victim, Character *killer )
{
    aquest_trigger(victim, killer, "Death", "CC", victim, killer);
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


/*
 * Main death handler.
 */ 
void raw_kill( Character* victim, bitstring_t flags, Character* ch, const DLString &label, int damtype )
{
    Character *realKiller = ch;

    // Transfer the killer flag to master if killer is a charmed npc.
    if (ch && ch->is_npc() && ch->master && !ch->master->is_npc() && ch != victim)
        ch = ch->master;

    stop_fighting( victim, true );
    victim->unsetLastFightTime( );
    
    // onDeath obj trigger for carried item can interrupt normal death handling.
    if (oprog_death( victim, ch ))
        return;
    
    if (ch)
        mprog_kill( ch, victim );

    // onDeath mob trigger can interrupt normal death handling.
    if (mprog_death( victim, ch ))
        return;

    // From this point on the death has certainly happened.

    gprog("onDeath", "CCisi", victim, ch, -1, label.c_str(), damtype);

    DeathPenalties(ch, victim).run();

    // Experience gain!
    if (ch)
        group_gain(ch, victim, realKiller);

    if (!IS_SET(flags, DEATH_MOB_EXTRACT))
        make_corpse( ch, victim ); // TO-DO: move this to fenia too

    // MOB is killed.
    if (victim->is_npc( )) {
        killed_npc_gain( ch, victim->getNPC( ) );

        if (IS_SET(flags, DEATH_MOB_EXTRACT)) { // full extract w/o trace, e.g. disintegrate
            extract_char(victim, true);
            return;
        }

        victim->setDead( );
        DeathAutoCommands( ch, victim ).run( );   
        return;
    }

    // PLAYER is killed.

    // Notify various attributes about player death, e.g. quest data.
    victim->getPC( )->getAttributes( ).handleEvent( DeathArguments( victim->getPC( ), ch ) );

    // Reset player and move them to the altar.    
    extract_dead_player( victim->getPC( ), FEXTRACT_COUNT );
    
    loyalty_gain( ch, victim );

    ghost_gain( victim );
    
    corpse_reequip( victim );

    // Player kills another player: flag and clan stats gain.
    if (ch && !ch->is_npc() && ch != victim) {
        set_killer( ch );
        set_slain( victim );
        ch->getClan( )->handleVictory( ch->getPC( ), victim->getPC( ) );
        victim->getClan( )->handleDefeat( victim->getPC( ), ch->getPC( ) );
        ch->getPC()->save();
    }

    victim->getPC()->save();
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

