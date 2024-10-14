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
#include "commandtemplate.h"
#include "liquid.h"
#include "desire.h"
#include "room.h"
#include "roomutils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "affect.h"
#include "skillreference.h"

#include "areaquestutils.h"
#include "loadsave.h"
#include "immunity.h"
#include "damageflags.h"
#include "save.h"
#include "interp.h"
#include "fight.h"
#include "act.h"
#include "merc.h"

#include "vnum.h"
#include "def.h"
#include "magic.h"

PROF(vampire);
GSN(none);
GSN(poison);
GSN(chill_touch);
GSN(poured_liquid);
RACE(satyr);
LIQ(none);
LIQ(water);
LIQ(valerian_tincture);
LIQ(swill);

/*
 * 'fill' command
 */
CMDRUN( fill )
{
    Object *obj;
    int amount = 0;
    Object *source = 0;
    Liquid *liq;
    DrinkContainer::Pointer drink;
    Room *room = ch->in_room;
    DLString arguments = constArguments, arg, arg1;

    arg = arguments.getOneArgument( );
    arg1 = arguments.getOneArgument( );

    if (arg.empty( )) {
        ch->pecho("Наполнить что?");
        return;
    }

    if (( obj = get_obj_wear_carry( ch, arg.c_str( ), 0 ) ) == 0) {
        ch->pecho("У тебя нет этого.");
        return;
    }

    if (obj->item_type == ITEM_FOUNTAIN) {
        ch->pecho("Чтобы наполнить фонтан, попробуй вылить туда емкость с жидкостью.", obj);
        return;
    }
	
    if (obj->item_type != ITEM_DRINK_CON) {
        ch->pecho( "Ты не можешь наполнить %O4 -- это не емкость для жидкости.", obj );
        return;
    }

    if (arg1.empty( )) {
        source = get_obj_room_type( ch, ITEM_FOUNTAIN );
		if (!source) source = get_obj_room_type( ch, ITEM_DRINK_CON );
        if (!source && room->getLiquid() == liq_none)
            if (!IS_SET(ch->in_room->room_flags, ROOM_NEAR_WATER)) {
                ch->pecho("Здесь нет источника!");
                return;
            }
    }
    else {
        source = get_obj_here( ch, arg1.c_str( ) );
        if (!source) {
            ch->pecho("Здесь нет такого источника.");
            return;
        }

        if (source->item_type != ITEM_FOUNTAIN &&
		   	source->item_type != ITEM_DRINK_CON) {
            ch->pecho("%1$^O1 -- не фонтан и не емкость для жидкости.", source);
            return;
        } 
    }

    if (drink_is_closed( obj, ch ))
        return;

	// Source is empty or frozen
    if (source && source->value1() == 0) {
		if (source->isAffected(gsn_chill_touch))
			ch->pecho("Жидкость в %1$O6, похоже, заморожена.", source);
		else ch->pecho("В %1$O6, похоже, пусто.", source);
        return;
    }
	
    if (source)
        liq = liquidManager->find( source->value2() );
    else if (room->getLiquid() != liq_none)
        liq = room->getLiquid().getElement();
    else
        liq = liq_water.getElement();

    const char *liqname = liq->getShortDescr().c_str();

    if (obj->value1() > 0 && obj->value2() != liq->getIndex()) {
        ch->pecho("Ты пытаешься наполнить %O4 %N5, но туда уже налита другая жидкость.", 
                  obj, liqname);
        ch->pecho("Ее придется сначала вылить.", obj);
		return;
    }

    if (obj->value1() >= obj->value0()) {
        ch->pecho("%1$^O1 уже наполнен%1$Gо||а|ы до краев.", obj);
        return;
    }    
    
    if (source) {
        ch->pecho( "Ты наполняешь %O4 %N5 из %O2.", obj, liqname, source );
        ch->recho( "%^C1 наполняет %O4 %N5 из %O2.",ch, obj, liqname, source );
    } else {
        ch->pecho("Ты зачерпываешь %N4 и наполняешь %O4.", liqname, obj);
        ch->recho("%^C1 зачерпывает %N4 и наполняет %O4.", ch, liqname, obj);
    }

    if (source && source->value0() > -1) {
        amount = obj->value0() - obj->value1();
        amount = min(amount, source->value1());
        obj->value2(liq->getIndex());
        obj->value1(obj->value1() + amount);
        source->value1(source->value1() - amount);
    }
    else {
        obj->value2(liq->getIndex());
        obj->value1(obj->value0());
    }

    // If the source was poisoned, poison the drink container as well
    if (source && (IS_SET( source->value3(), DRINK_POISONED ) || source->isAffected(gsn_poison)))
    	obj->value3(obj->value3() | DRINK_POISONED);
	
    if (obj->behavior && ( drink = obj->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->fill( ch, source, amount ); // source can be null here
}

/*
 * 'pour' command
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
    DLString liqShort, liqName;
    Room *room = out->getRoom();

    time = amount / 15;    
    if (time == 0) 
        time = 1;
    else 
        time = std::max( 2, time ); 

    pool = get_obj_room_vnum( room, OBJ_VNUM_POOL ); // 75
	liquid = liquidManager->find( out->value2() );	
    liqShort = liquid->getShortDescr( );
    liqName  = liquid->getName( );
	
	if (pool) {
		amount = pool->value1() + amount;
		if (out->value2() != pool->value2()) {
			liquid = liquidManager->find(liq_swill); // When liquids mix, make "swill"
    		liqShort = liquid->getShortDescr( ); // reassign names to swill
    		liqName  = liquid->getName( );			
			room->echo( POS_RESTING, "%^N1 и %N1 смешиваются, образуя лужу %N2.",
					   liquidManager->find(out->value2())->getShortDescr( ).c_str( ),
					   liquidManager->find(pool->value2())->getShortDescr( ).c_str( ),
					   liqShort.c_str( ) );
		}
		else room->echo( POS_RESTING, "Лужа %N2 растекается еще шире.", liqShort.c_str( ) );
	}
    else {
		pool = create_object(get_obj_index(OBJ_VNUM_POOL), 0);
		room->echo( POS_RESTING, "На земле образуется лужа %N2.", liqShort.c_str( ) );
	}
	
    pool->setShortDescr( fmt(0, pool->pIndexData->short_descr, liqShort.ruscase( '2' ).c_str( )) );
    pool->setDescription( fmt(0, pool->pIndexData->description, liqShort.ruscase( '2' ).c_str( )) );
	// Don't set material to liquid name -- those don't exist in the materials list
	// Use "drink" material for unspecified liquid types (set by default in OBJ_VNUM_POOL, vnum 75)
    // pool->setMaterial( liqName.c_str() );        
        
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
        room->echo(POS_RESTING, "Из %O2 не выливается ни капли.", out);
        return;
    }

    amount = out->value1();
    out->value1(0);
    out->value3(0);

    Liquid *liq = liquidManager->find(out->value2());
    const char *liqname = liq->getName().c_str();
    DLString liqShort = liq->getShortDescr();
    Character *ch = out->carried_by;

    if (ch && oprog_empty(out, ch, liqname, amount))
        return;

    if (ch) {
        ch->pecho("Ты переворачиваешь %O4.", out);
        ch->recho("%^C1 переворачивает %O4.", ch, out);
    }

    if (RoomUtils::isWater(room))
        room->echo(POS_RESTING, "Поток %N2 из %O2 выплескивается в %N4.", liqShort.c_str(), out, room->getLiquid()->getShortDescr().c_str());
    else if (room->getSectorType() == SECT_AIR)
        room->echo(POS_RESTING, "Поток %N2 из %O2 устремляется куда-то вниз и пропадает.", liqShort.c_str(), out); // TO-DO: move to non-air room downwards
    else if (room->getSectorType() == SECT_DESERT)
        room->echo(POS_RESTING, "Лужа %N2 из %O2 с шипением испаряется на песке.", liqShort.c_str(), out);
    else {
        room->echo(POS_RESTING, "Поток %N2 из %O2 проливается на землю.", liqShort.c_str(), out);
        create_pool(out, amount);
    }

    if (ch && out->behavior && out->behavior.getDynamicPointer<DrinkContainer>())
        out->behavior.getDynamicPointer<DrinkContainer>()->pourOut(ch, amount);
}

static void oprog_pour_out( Object *obj, Character *ch, Object *out, const char *liqname, int amount )
{
    FENIA_VOID_CALL( obj, "PourOut", "COsi", ch, out, liqname, amount );
    FENIA_NDX_VOID_CALL( obj, "PourOut", "OCOsi", obj, ch, out, liqname, amount );

    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect())
            paf->type->getAffect()->onPourOut(SpellTarget::Pointer(NEW, obj), paf, ch, out, liqname, amount);
}

static void mprog_pour_out( Character *victim, Character *ch, Object *out, const char *liqname, int amount  )
{
    aquest_trigger(victim, ch, "PourOut", "CCOsi", victim, ch, out, liqname, amount);
    FENIA_VOID_CALL( victim, "PourOut", "COsi", ch, out, liqname, amount );
    FENIA_NDX_VOID_CALL( victim->getNPC( ), "PourOut", "CCOsi", victim, ch, out, liqname, amount );
}

void pour_out( Character *ch, Object * out, Character *victim )
{
    Liquid *liquid;
    int sips, amount;
    DLString msgRoom, msgVict, msgChar;
    DLString msgOther, msgSelf;

    liquid = liquidManager->find( out->value2() );
    amount = out->value1();
    sips = max( 1, amount / liquid->getSipSize( ) );

    if (out->value1() == 0) {
        msgChar = "Ты опрокидываешь на %2$C4 %3$O4, однако оттуда не выливается ни капли.";
        msgVict = "%1$^C1 переворачивает над тобой %3$O4, однако оттуда не выливается ни капли.";
        msgRoom = "%1$^C1 переворачивает над %2$C5 %3$O4, однако оттуда не выливается ни капли.";

        msgSelf = "Ты опрокидываешь на себя %3$O4, однако оттуда не выливается ни капли.";
        msgOther= "Приговаривая 'ну котеночек, ну еще капельку', %1$C1 переворачивает и трясет над головой %3$O4.";
    }
    else if (sips < 2) {
        msgChar = "Ты брызгаешь на %2$C4 %4$N5 из %3$O2.";
        msgVict = "%1$^C1 брызгает на тебя %4$N5 из %3$O2.";
        msgRoom = "%1$^C1 брызгает на %2$C4 %4$N5 из %3$O2.";

        msgSelf = "Ты брызгаешь на себя %4$N5 из %3$O2.";
        msgOther= "%1$^C1 брызгает на себя %4$N5 из %3$O2.";
    }
    else if (sips < 25) {
        msgChar = "Ты выливаешь на %2$C4 %4$N4 из %3$O2.";
        msgVict = "%1$^C1 выливает на тебя %4$N4 из %3$O2.";
        msgRoom = "%1$^C1 выливает на %2$C4 %4$N4 из %3$O2.";

        msgSelf = "Ты выливаешь на себя %4$N4 из %3$O2.";
        msgOther= "%1$^C1 выливает на себя %4$N4 из %3$O2.";
    }
    else {
        msgChar = "Ты опрокидываешь на %2$C4 %3$O4, с ног до головы обливая %2$P2 %4$N5!";
        msgVict = "%1$^C1 опрокидывает на тебя %3$O4, с ног до головы обливая тебя %4$N5!";
        msgRoom = "%1$^C1 опрокидывает на %2$C4 %3$O4, с ног до головы обливая %2$P2 %4$N5!";

        msgSelf = "Ты опрокидываешь на себя %3$O4, с ног до головы обливаясь %4$N5!";
        msgOther= "%1$^C1 опрокидывает на себя %3$O4, с ног до головы обливаясь %4$N5!";
    }
    
    if (ch == victim) {
        ch->pecho( msgSelf.c_str( ), ch, victim, out, liquid->getShortDescr( ).c_str( ) );
        ch->recho( msgOther.c_str( ), ch, victim, out, liquid->getShortDescr( ).c_str( ) );
    }
    else {
        ch->pecho( msgChar.c_str( ), ch, victim, out, liquid->getShortDescr( ).c_str( ) );
        ch->recho( victim, msgRoom.c_str( ), ch, victim, out, liquid->getShortDescr( ).c_str( ) );

        if (IS_AWAKE(victim)) {
            victim->pecho( msgVict.c_str( ), ch, victim, out, liquid->getShortDescr( ).c_str( ) );
        }
        else if (sips >= 5) {
            victim->pecho( "Ты чувствуешь влагу на теле." );

            if (!IS_AFFECTED(victim, AFF_SLEEP))
                interpret_raw( ch, "wake", 
                               get_char_name_list( victim, ch->in_room->people, ch ).c_str( ) );
        }
    }
    
    if (out->value1() == 0)
        return;

    out->value1(0);
    out->value3(0);
    
    if (sips >= 5) {
        if (liq_water == liquid) {
            bool rc = false;

            for (auto &paf: victim->affected.findAll(gsn_poured_liquid)) {
                paf->duration -= sips / 5;
                rc = true;

                if (paf->duration < 0) 
                    affect_remove( victim, paf );
            }

            if (rc) 
                victim->pecho( POS_RESTING, "Вода смывает с тебя часть посторонних запахов." );
        }
        else {
            Affect af;
            
            af.type = gsn_poured_liquid;
            af.duration = sips / 5;
            af.global.setRegistry( liquidManager );
            af.global.set( liquid->getIndex( ) );
            affect_join( victim, &af );
        }
    }

    if (out->behavior && out->behavior.getDynamicPointer<DrinkContainer>( ))
        out->behavior.getDynamicPointer<DrinkContainer>( )->pourOut( ch, victim, amount );
    
    const char *liqname = liquid->getName( ).c_str( );

    mprog_pour_out( victim, ch, out, liqname, amount );

    for (Object *obj = victim->carrying; obj; obj = obj->next_content)
        oprog_pour_out( obj, ch, out, liqname, amount );
}

static void pour_in( Character *ch, Object *out, Object *in, Character *vch )
{
    DrinkContainer::Pointer drink;

	////// Check the source (out) eligibility first
	
	// Source is empty or frozen
    if (out->value1() == 0) {
		if (out->isAffected(gsn_chill_touch))
			ch->pecho("Жидкость в %1$O6, похоже, заморожена.", out);
		else ch->pecho("В %1$O6, похоже, пусто.", out);
        return;
    }
	
	////// Now check the target (in) eligibility
	
	// Check for correct item type 
    if (in->item_type != ITEM_DRINK_CON && in->item_type != ITEM_FOUNTAIN) {
        if (!vch || vch == ch)
            ch->pecho("Ты пытаешься налить что-то в %O4, а это не фонтан и не емкость для жидкости.", in);
        else
            ch->pecho("В руках у %C2 зажата совсем не емкость для жидкости.", vch);
        return;
    }
	
    if (in == out) {
        ch->pecho("Ты не можешь перелить из %1$O2 в сам%1$Gое|ого|у|их себя!", in);
        return;
    }
	
	// Check if closed (fountains can't be closed)
    if (in->item_type == ITEM_DRINK_CON) {
		if (drink_is_closed( in, ch ))
			return;
	}
	
	// Check if target is frozen
	if (in->isAffected(gsn_chill_touch)) {
		ch->pecho("Жидкость в %1$O6, похоже, заморожена -- придется сначала разморозить.", in);
		return;
	}
	
	// Don't fill infinite fountains
    if (in->value0() < 0) {
		ch->pecho("%1$^O1 бездон%1$Gно|eн|на|ны и не нужда%1$nется|ются в заполнении.", in);
        return;
    }
	
    if (in->value1() > 0 && in->value1() == in->value0()) {
        if (!vch || vch == ch)
            ch->pecho( "%1$^O1 уже полностью заполне%1$Gно|н|на|ны.", in );
        else
            ch->pecho( "%1$^O1 в руках у %2$C2 уже полностью заполне%1$Gно|н|на|ны.", in, vch );
        return;
    }
	
	// Can't mix liquids (yet!)
    if (in->value1() != 0 && in->value2() != out->value2()) {
        if (!vch || vch == ch)
            ch->pecho("В %O4 налита другая жидкость.", in);
        else
            ch->pecho("В %O4 в руках у %C2 налита другая жидкость.", in, vch);
        return;
    }

    int amount = min(out->value1(),in->value0() - in->value1());

    in->value1(in->value1() + amount);
    out->value1(out->value1() - amount);
    in->value2(out->value2());

    Liquid *liq = liquidManager->find( out->value2() );
    const char *liqShort = liq->getShortDescr( ).c_str( );

    if (vch == 0) {
        ch->pecho( "Ты наливаешь %N4 из %O2 в %O4.", liqShort, out, in );
        ch->recho( "%^C1 наливает %N4 из %O2 в %O4.", ch, liqShort, out, in );
    }
    else {
        if (vch != ch) {
            ch->pecho( "Ты наливаешь %N4 для %C2.", liqShort, vch );
            vch->pecho( "%^C1 наливает тебе %N4.", ch, liqShort );
            ch->recho( vch, "%^C1 наливает %N4 для %C2.", ch, liqShort, vch );
        }
        else {
            ch->pecho( "Ты наливаешь себе %N4.", liqShort );
            ch->recho( "%^C1 наливает себе %N4.", ch, liqShort );
        }
    }

    if (in->behavior && ( drink = in->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->fill( ch, out, amount );

    if (out->behavior && ( drink = out->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->pour( ch, in, amount );
}

/**
 * Syntax:
 * pourout|вылить <drink1>
 * pourout|вылить <drink1> <victim>
 */
CMDRUN( pourout )
{
    DLString arg1, arg2;
    DLString arguments = constArguments;
    Object *out;
    Character *vch = 0;
    
    arg1 = arguments.getOneArgument( ); // drink1
    arg2 = arguments.getOneArgument( ); // victim or empty

    if (arg1.empty( )) {
        ch->pecho("Вылить что и куда?");
        return;
    }

    if ((out = get_obj_carry(ch,arg1.c_str( ))) == 0) {
        ch->pecho("У тебя в инвентаре нет такой емкости для жидкости.");
        return;
    }

    if (out->item_type != ITEM_DRINK_CON) {
        ch->pecho("%^O1 -- не емкость для жидкости.", out);
        return;
    }

    if (drink_is_closed( out, ch ))
        return;
    
    if (arg2.empty( )) {
        pour_out( out );
        return;
    }

    if (( vch = get_char_room(ch, arg2 ) ) == 0) {
        ch->pecho( "Вылить на кого?" );
        return;
    }

    pour_out( ch, out, vch );
}

/**
 * Syntax:
 * pour <drink1> <drink2>
 * pour <drink1> <victim>
 * pour <drink1> out
 * pour <drink1> out <victim>
 */
CMDRUN( pour )
{
    DLString arg1, arg2, arg3;
    DLString arguments = constArguments;
    Object *out, *in;
    Character *vch = 0;
    
    arg1 = arguments.getOneArgument( ); // drink1
    arg2 = arguments.getOneArgument( ); // drink2/fountain or victim or "out" or empty
    arg3 = arguments.getOneArgument( ); // victim (holding drink2) or empty

    if (arg1.empty( )) {
        ch->pecho("Вылить что и куда?");
        return;
    }

    if ((out = get_obj_carry(ch,arg1.c_str( ))) == 0) {
        ch->pecho("У тебя в инвентаре нет такой емкости для жидкости.");
        return;
    }

    if (out->item_type != ITEM_DRINK_CON) {
        ch->pecho("%^O1 -- не емкость для жидкости.", out);
        return;
    }

    if (drink_is_closed( out, ch ))
        return;
    
    if (arg2 == "out" || arg2.empty( )) {
        if (!arg3.empty( )) {
            if (( vch = get_char_room(ch, arg3 ) ) == 0) {
                ch->pecho( "Вылить на кого?" );
                return;
            }

            pour_out( ch, out, vch );
        }
        else
            pour_out( out );

        return;
    }

    if ((in = get_obj_here(ch, arg2)) == 0) {
        vch = get_char_room(ch, arg2);

        if (vch == 0) {
            ch->pecho("Вылить во что?");
            return;
        }

        in = wear_hold->find( vch );

        if (in == 0) {
            ch->pecho("У %C2 в руках нет бокала или другой емкости.", vch);
            return;
        }
    }

    pour_in(ch, out, in, vch);
}

/*
 * 'drink' command
 */
/** Call prog for drinking character. */
static bool mprog_drink( Character *ch, Object *obj, const char *liq, int amount )
{
    FENIA_CALL(ch, "Drink", "Osi", obj, liq, amount);
    FENIA_NDX_CALL(ch->getNPC( ), "Drink", "COsi", ch, obj, liq, amount);
    return false;
}

/** Call prog for drinking container */
static bool oprog_drink( Object *obj, Character *ch, const char *liq, int amount )
{
    FENIA_CALL( obj, "Drink", "Csi", ch, liq, amount );
    FENIA_NDX_CALL( obj, "Drink", "OCsi", obj, ch, liq, amount );
    return false;
}

/** Call prog for a carried/worn item. */
static bool oprog_drink_near( Object *obj, Object *drink, Character *ch, const char *liq, int amount )
{
    FENIA_CALL( obj, "DrinkNear", "OCsi", drink, ch, liq, amount );
    FENIA_NDX_CALL( obj, "DrinkNear", "OOCsi", obj, drink, ch, liq, amount );
    return false;
}

/** Called for everyone around the drinker. */
static bool mprog_drink_near( Character *drinker, Object *obj, const char *liq, int amount )
{
    for (auto &rch: drinker->in_room->getPeople()) {
        if (rch != drinker) {
            FENIA_CALL(rch, "DrinkNear", "OCsi", obj, drinker, liq, amount);
            FENIA_NDX_CALL(rch->getNPC( ), "DrinkNear", "COCsi", rch, obj, drinker, liq, amount);
        }
    }

    return false;
}

CMDRUN( drink )
{
    Object *obj;
    int amount = 0;
    Liquid *liquid;
    DrinkContainer::Pointer drink;
    RoomIndexData *pRoom = ch->in_room->pIndexData;
    DLString arguments = constArguments, arg;
    
    arg = arguments.getOneArgument( );

    if (arg.empty( )) {
        obj = get_obj_room_type( ch, ITEM_FOUNTAIN );
        if (!obj && pRoom->liquid == liq_none) 
            if (!IS_SET(ch->in_room->room_flags, ROOM_NEAR_WATER)) {
                ch->pecho("Выпить что?");
                return;
            }
    }
    else {
        if (( obj = get_obj_here( ch, arg ) ) == 0) {
            ch->pecho("Ты не находишь этого.");
            return;
        }
    }

    if (obj) {
        switch (obj->item_type) {
        default:
            ch->pecho("Ты не можешь пить из %O2.", obj);
            return;

        case ITEM_FOUNTAIN:
			// Source is empty or frozen
    		if (obj->value0() > -1 && obj->value1() == 0) {
				if (obj->isAffected(gsn_chill_touch))
					ch->pecho("Жидкость в %1$O6, похоже, заморожена.", obj);
				else ch->pecho("В %1$O6, похоже, пусто.", obj);
        		return;
    		}			

            liquid = liquidManager->find( obj->value2() );
            amount = liquid->getSipSize( ) * 3;
            amount = min(amount, obj->value1());
            break;

        case ITEM_DRINK_CON:
            if (drink_is_closed( obj, ch ))
                return;

			// Source is empty or frozen
    		if (obj->value1() <= 0) {
				if (obj->isAffected(gsn_chill_touch))
					ch->pecho("Жидкость в %1$O6, похоже, заморожена.", obj);
				else ch->pecho("В %1$O6, похоже, пусто.", obj);
        		return;
    		}

            liquid = liquidManager->find( obj->value2() );
            amount = liquid->getSipSize( );
            amount = min(amount, obj->value1());
            break;
        }
    } else if (pRoom->liquid != liq_none) {
        liquid = pRoom->liquid.getElement();
        amount = liquid->getSipSize( ) * 3;
    } else {
        liquid = liq_water.getElement();
        amount = liquid->getSipSize( ) * 3;
    }
    
    if (!ch->is_npc( ))
        for (int i = 0; i < desireManager->size( ); i++)
            if (!desireManager->find( i )->canDrink( ch->getPC( ) ))
                return;
    
    DLString buf = liquid->getShortDescr( ).ruscase( '4' );

    if (obj) {
        oldact("$c1 пьет $T из $o2.", ch,obj,buf.c_str( ),TO_ROOM );
        oldact("Ты пьешь $T из $o2.", ch,obj,buf.c_str( ),TO_CHAR );
    } else {
        oldact("$c1 зачерпывает и пьет $T.", ch, 0, buf.c_str( ),TO_ROOM );
        oldact("Ты зачерпываешь и пьешь $T.", ch, 0, buf.c_str( ),TO_CHAR );
    }

    if (ch->fighting != 0)
         ch->setWaitViolence( 3 );
    
    if (!ch->is_npc( )) 
        for (int i = 0; i < desireManager->size( ); i++)
            desireManager->find( i )->drink( ch->getPC( ), amount, liquid );

    /* The drink was poisoned ! */
    if (obj && (IS_SET( obj->value3(), DRINK_POISONED ) || obj->isAffected(gsn_poison)))
    {
        int level = number_fuzzy(amount);
        Affect af;

        if ( !saves_spell(level / 2, ch, DAM_POISON) ) {
            ch->recho("%1$^C4 начинает тошнить, когда яд проникает в %1$Gего|его|ее|их тел%1$nо|а.", ch);
            ch->pecho("Тебя начинает тошнить, когда яд проникает в твое тело.");

            af.bitvector.setTable(&affect_flags);
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = 3 * amount;
            af.bitvector.setValue(AFF_POISON);
            affect_join( ch, &af );
        }
    }

    if (obj && obj->value0() > -1)
        obj->value1(obj->value1() - amount);

    if (obj && obj->behavior && ( drink = obj->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->drink( ch, amount );

    if (mprog_drink( ch, obj, liquid->getName( ).c_str( ), amount ))
        return;

    if (obj && oprog_drink( obj, ch, liquid->getName( ).c_str( ), amount ))
        return;

    for (Object *o = ch->carrying; o; o = o->next_content)
        if (oprog_drink_near(o, obj, ch, liquid->getName().c_str(), amount))
            return;

    if (mprog_drink_near( ch, obj, liquid->getName( ).c_str( ), amount ))
        return;

    if (obj && IS_OBJ_STAT(obj, ITEM_BLESS) && IS_EVIL(ch) && immune_check(ch, DAM_HOLY, DAMF_OTHER) == RESIST_VULNERABLE) {
        ch->pecho("Святость %O2 обжигает твои внутренности!", obj);
        ch->recho("Лицо %^C2 искажается гримасой боли.", ch);
        rawdamage(ch, ch, DAM_HOLY, ch->hit / 100 + 1, true, "holywater");
    }
}


/*---------------------------------------------------------------------------
 * smell affects
 *--------------------------------------------------------------------------*/
bool oprog_smell_liquid(Liquid *liq, Character *sniffer)
{
        DLString msg;

        if (liq->getFlags( ).isSet( LIQF_LIQUOR ))
            msg = "Тебе в нос ударяет резкий запах %1$N2.";
        else if (liq->getFlags( ).isSet( LIQF_WINE ))
            msg = "Ты улавливаешь аромат %1$N2.";
        else if (liq->getFlags( ).isSet( LIQF_BEER ))
            msg = "Пахнет %1$N5.";
        else if (liq->getIndex() != liq_water)
            msg = "Ты чувствуешь запах %1$N2.";
        else
            return false;

        sniffer->pecho( msg.c_str( ), liq->getShortDescr( ).c_str( ) );
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


