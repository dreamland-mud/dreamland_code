/* $Id$
 *
 * ruffina, 2004
 */
#include "drink_utils.h"
#include "drinkcontainer.h"
#include "liquidflags.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "affecthandler.h"
#include "affecthandlertemplate.h"
#include "liquid.h"
#include "room.h"
#include "roomutils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "affect.h"
#include "skillreference.h"

#include "loadsave.h"
#include "../loadsave/behavior_utils.h"
#include "damageflags.h"
#include "save.h"
#include "player_utils.h"
#include "act.h"
#include "merc.h"

#include "vnum.h"
#include "def.h"
#include "l10n.h"

GSN(poison);
RACE(satyr);
LIQ(water);
LIQ(valerian_tincture);
LIQ(swill);

/*
 * Pouring a container out onto the ground: pour_out(Object*), bound to Fenia as
 * obj.pour_out(). The drink/fill/pour/pourout commands themselves are Fenia
 * CommandElements (command/<name>/runFunc).
 */
static bool oprog_empty( Object *obj, Character *ch, const char *liqname, int amount )
{
    FENIA_CALL( obj, "Empty", "Csi", ch, liqname, amount );
    FENIA_NDX_CALL( obj, "Empty", "OCsi", obj, ch, liqname, amount );
    return false;
}

static void create_pool( Object *out, int amount ) 
{
    Object *pool;
    int time;
    Liquid *liquid;
    DLString liqName;
    Room *room = out->getRoom();

    time = amount / 15;    
    if (time == 0) 
        time = 1;
    else 
        time = std::max( 2, time ); 

    pool = get_obj_room_vnum( room, OBJ_VNUM_POOL ); // 75
	liquid = liquidManager->find( out->value2() );
    liqName  = liquid->getName( );

	// Room::echo resolves the FORMAT per listener but shares one argument list, so a
	// pre-rendered liquid noun reaches every viewer in a single language -- which is
	// how an English sentence ended up saying "A puddle of пива". Loop the room and
	// render the noun in each listener's own language instead.
	if (pool) {
		amount = pool->value1() + amount;
		if (out->value2() != pool->value2()) {
			Liquid *poured = liquidManager->find(out->value2());
			Liquid *pooled = liquidManager->find(pool->value2());

			liquid = liquidManager->find(liq_swill); // When liquids mix, make "swill"
    		liqName  = liquid->getName( ); // reassign name to swill

			for (Character *rch = room->people; rch; rch = rch->next_in_room) {
				if (rch->position < POS_RESTING)
					continue;
				lang_t lg = Player::lang(rch);
				rch->pecho( l(rch, "%^N1 и %N1 смешиваются, образуя лужу %N2."),
							poured->getShortDescr( lg ).c_str( ),
							pooled->getShortDescr( lg ).c_str( ),
							liquid->getShortDescr( lg ).c_str( ) );
			}
		}
		else {
			for (Character *rch = room->people; rch; rch = rch->next_in_room) {
				if (rch->position < POS_RESTING)
					continue;
				rch->pecho( l(rch, "Лужа %N2 растекается еще шире."),
							liquid->getShortDescr( Player::lang(rch) ).c_str( ) );
			}
		}
	}
    else {
		pool = create_object(get_obj_index(OBJ_VNUM_POOL), 0);

		for (Character *rch = room->people; rch; rch = rch->next_in_room) {
			if (rch->position < POS_RESTING)
				continue;
			rch->pecho( l(rch, "На земле образуется лужа %N2."),
						liquid->getShortDescr( Player::lang(rch) ).c_str( ) );
		}
	}
	
    // Build the puddle name/description per viewer language: the pool prototype
    // templates carry a %s for the liquid noun in every language and the liquid
    // itself has per-language shorts, so fill every slot instead of RU only --
    // otherwise EN/UA viewers fall back to the empty/prototype puddle name.
    // RU byte-identical (LANG_DEFAULT slot renders exactly as before).
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        DLString ln = liquid->getShortDescr( lang ).ruscase( '2' );
        pool->setShortDescr( fmt(0, pool->pIndexData->short_descr.get(lang).c_str(), ln.c_str( )), lang );
        pool->setDescription( fmt(0, pool->pIndexData->description.get(lang).c_str(), ln.c_str( )), lang );
    }
	// Don't set material to liquid name -- those don't exist in the materials list
	// Use "drink" material for unspecified liquid types (set by default in OBJ_VNUM_POOL, vnum 75)
    // pool->setMaterial( liqName );        
        
    pool->timer += time;
    pool->value0(max( 1, amount )); // Puddle can't be "broader" than its contents
	pool->value1(max( 1, amount ));
	pool->value2(liquid->getIndex( ));

    if (!pool->in_room) 
        obj_to_room(pool, room);
    else 
        save_items(room);
}

void pour_out(Object *out)
{
    int amount;
    Room *room = out->getRoom();

    // Tai: updating this to include the destruction of items, not just manual pour out

    if (out->value1() == 0) {
        room->echo(POS_RESTING, _("Из %O2 не выливается ни капли."), out);
        return;
    }

    amount = out->value1();
    out->value1(0);
    out->value3(0);
    if (out->item_type == ITEM_DRINK_CON)
        affect_strip( out, gsn_poison );

    Liquid *liq = liquidManager->find(out->value2());
    const char *liqname = liq->getName().c_str();
    Character *ch = out->carried_by;

    if (ch && oprog_empty(out, ch, liqname, amount))
        return;

    if (ch) {
        ch->pecho(_("Ты переворачиваешь %O4."), out);
        ch->recho(_("%^C1 переворачивает %O4."), ch, out);
    }

    // Decide the destination once, so the message and the puddle can never disagree.
    bool intoWater = RoomUtils::isWater(room);
    bool intoAir   = !intoWater && room->getSectorType() == SECT_AIR;
    bool intoSand  = !intoWater && !intoAir && room->getSectorType() == SECT_DESERT;
    bool onGround  = !intoWater && !intoAir && !intoSand;

    // Same per-listener loop as create_pool: the liquid noun has to be rendered in
    // each viewer's language, because the shared argument list cannot vary with the
    // format Room::echo picks for them.
    for (Character *rch = room->people; rch; rch = rch->next_in_room) {
        if (rch->position < POS_RESTING)
            continue;

        lang_t lg = Player::lang(rch);
        const char *ln = liq->getShortDescr( lg ).c_str( );

        if (intoWater)
            rch->pecho(l(rch, "Поток %N2 из %O2 выплескивается в %N4."),
                       ln, out, room->getLiquid()->getShortDescr( lg ).c_str( ));
        else if (intoAir)
            rch->pecho(l(rch, "Поток %N2 из %O2 устремляется куда-то вниз и пропадает."), ln, out); // TO-DO: move to non-air room downwards
        else if (intoSand)
            rch->pecho(l(rch, "Лужа %N2 из %O2 с шипением испаряется на песке."), ln, out);
        else
            rch->pecho(l(rch, "Поток %N2 из %O2 проливается на землю."), ln, out);
    }

    if (onGround) {
        // Notify every object already in the room that liquid was poured over
        // it. Two receiving-side channels, both new for a ground pour: a bedit
        // behavior (PourOn) and each affect's onPourOut. The latter is what an
        // incandescent item hisses out on -- water poured on the floor where a
        // red-hot thing lies (Venzdey / Trello 2633). That handler was written
        // for "poured onto" but nothing fired it in that direction until now.
        // Snapshot next first: a reacting handler may extract the item.
        Object *next;
        for (Object *item = room->contents; item; item = next) {
            next = item->next_content;
            behavior_trigger( item, "PourOn", "OCOsi", item, ch, out, liqname, amount );

            for (auto &paf: item->affected.findAllWithHandler())
                if (paf->type->getAffect())
                    paf->type->getAffect()->onPourOut(SpellTarget::Pointer(NEW, item), paf, ch, out, liqname, amount);
        }

        create_pool(out, amount);
    }

    if (ch && out->behavior && out->behavior.getDynamicPointer<DrinkContainer>())
        out->behavior.getDynamicPointer<DrinkContainer>()->pourOut(ch, amount);

    // Source-container hook for dumping the drink on the GROUND (no target).
    // Distinct name from "PourVict" (character target) and "PourOut" (poured into
    // a destination object): this one carries neither. Lets an arcadian-enchanted
    // drink react when tipped out -- e.g. a beer elemental rising from the splash.
    // Guarded on ch: a carried-by pourer, mirroring the C++ behavior above. The
    // create_pool above already ran, so a Fenia handler can find the fresh pool.
    // Additive no-op until a Fenia handler exists.
    if (ch) {
        behavior_trigger( out, "PourGround", "OCsi", out, ch, liqname, amount );
        FENIA_VOID_CALL( out, "PourGround", "Csi", ch, liqname, amount );
        FENIA_NDX_VOID_CALL( out, "PourGround", "OCsi", out, ch, liqname, amount );
    }
}


/*---------------------------------------------------------------------------
 * smell affects
 *--------------------------------------------------------------------------*/
bool oprog_smell_liquid(Liquid *liq, Character *sniffer)
{
        DLString msg;
        lang_t lang = Player::lang(sniffer);

        if (liq->getFlags( ).isSet( LIQF_LIQUOR ))
            msg = lmsg(lang,
                "A sharp reek of %1$N2 hits your nose.",
                "Тебе в нос ударяет резкий запах %1$N2.",
                "Тобі в ніс б'є різкий запах %1$N2.");
        else if (liq->getFlags( ).isSet( LIQF_WINE ))
            msg = lmsg(lang,
                "You catch the aroma of %1$N2.",
                "Ты улавливаешь аромат %1$N2.",
                "Ти вловлюєш аромат %1$N2.");
        else if (liq->getFlags( ).isSet( LIQF_BEER ))
            msg = lmsg(lang,
                "It smells of %1$N5.",
                "Пахнет %1$N5.",
                "Пахне %1$N5.");
        else if (liq->getIndex() != liq_water)
            msg = lmsg(lang,
                "You smell %1$N2.",
                "Ты чувствуешь запах %1$N2.",
                "Ти відчуваєш запах %1$N2.");
        else
            return false;

        sniffer->pecho( msg.c_str( ), liq->getShortDescr( lang ).c_str( ) );
        return true;
}

AFFECT_DECL(PouredLiquid);
TYPE_AFFECT(bool, PouredLiquid)::smell( Character *ch, Character *sniffer, Affect *paf ) 
{
    bool rc = false;
    vector<int> bits = paf->global.toArray( );

    for (unsigned int i = 0; i < bits.size( ); i++) {
        Liquid *liq = liquidManager->find( bits[i] );
        if (oprog_smell_liquid(liq, sniffer))
            rc = true;
    }

    return rc;
}

VOID_AFFECT(PouredLiquid)::saves( Character *ch, Character *victim, int &save, int dam_type, Affect *paf ) 
{
    bool hasVuln = false;
    
    if (dam_type != DAM_CHARM)
        return;

    if (IS_SET( victim->form, FORM_FELINE )) { 
        if (paf->global.isSet( liq_valerian_tincture )) 
            hasVuln = true;
    }
    else if (victim->getRace( ) == race_satyr) {
        vector<int> bits = paf->global.toArray( );

        for (unsigned int i = 0; i < bits.size( ); i++) {
            Liquid *liq = liquidManager->find( bits[i] );

            if (liq->getFlags( ).isSet( LIQF_BEER|LIQF_WINE )) {
                hasVuln = true;
                break;
            }
        }
    }

    if (hasVuln) 
        save -= 5;
}


