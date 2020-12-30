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

#include "affecthandlertemplate.h"
#include "commandtemplate.h"
#include "liquid.h"
#include "desire.h"
#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "affect.h"
#include "skillreference.h"

#include "loadsave.h"
#include "immunity.h"
#include "damageflags.h"
#include "save.h"
#include "interp.h"
#include "fight.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

WEARLOC(hold);
PROF(vampire);
GSN(none);
GSN(poison);
GSN(poured_liquid);
RACE(felar);
RACE(cat);
RACE(satyr);
LIQ(none);
LIQ(water);
LIQ(valerian_tincture);


/*
 * 'fill' command
 */
CMDRUN( fill )
{
    Object *obj;
    int amount;
    Object *fountain = 0;
    Liquid *liq;
    DrinkContainer::Pointer drink;
    RoomIndexData *pRoom = ch->in_room->pIndexData;
    DLString arguments = constArguments, arg, arg1;

    arg = arguments.getOneArgument( );
    arg1 = arguments.getOneArgument( );

    if (arg.empty( )) {
        ch->send_to("Наполнить что?\n\r");
        return;
    }

    if (( obj = get_obj_wear_carry( ch, arg.c_str( ) ) ) == 0) {
        ch->send_to("У тебя нет этого.\n\r");
        return;
    }

    if (obj->item_type != ITEM_DRINK_CON) {
        ch->pecho( "Ты не можешь наполнить %O4.", obj );
        return;
    }

    if (arg1.empty( )) {
        fountain = get_obj_room_type( ch, ITEM_FOUNTAIN );
        if (!fountain && pRoom->liquid == liq_none)
            if (!IS_SET(ch->in_room->room_flags, ROOM_NEAR_WATER)) {
                ch->send_to("Здесь нет источника!\n\r");
                return;
            }
    }
    else {
        fountain = get_obj_here( ch, arg1.c_str( ) );
        if (!fountain) {
            ch->println("Здесь нет такого источника.");
            return;
        }

        if (fountain->item_type != ITEM_FOUNTAIN) {
            ch->pecho("%^O1 - не фонтан.", fountain);
            return;
        } 
    }

    if (drink_is_closed( obj, ch ))
        return;

    if (fountain)
        liq = liquidManager->find( fountain->value2() );
    else if (pRoom->liquid != liq_none)
        liq = pRoom->liquid.getElement();
    else
        liq = liq_water.getElement();

    const char *liqname = liq->getShortDescr().c_str();

    if (obj->value1() != 0 && obj->value2() != liq->getIndex()) {
        ch->pecho("Ты пытаешься наполнить %O4 %N5, но туда уже налита другая жидкость.", 
                  obj, liqname);
        return;
    }

    if (obj->value1() >= obj->value0()) {
        ch->pecho("%1$^O1 уже наполн%1$Gено|ен|на|ны до краев.", obj);
        return;
    }    
    
    if (fountain) {
        ch->pecho( "Ты наполняешь %O4 %N5 из %O2.", obj, liqname, fountain );
        ch->recho( "%^C1 наполняет %O4 %N5 из %O2.",ch, obj, liqname, fountain );
    } else {
        ch->pecho("Ты зачерпываешь %N4 и наполняешь %O4.", liqname, obj);
        ch->recho("%^C1 зачерпывает %N4 и наполняет %O4.", ch, liqname, obj);
    }

    amount = obj->value0() - obj->value1();
    obj->value2(liq->getIndex());
    obj->value1(obj->value0());

    if (obj->behavior && ( drink = obj->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->fill( ch, fountain, amount ); // fountain can be null here
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

static void create_pool( Character *ch, Object *out, int amount ) 
{
    Object *pool;
    int time;
    DLString liqShort;
    Room *room = ch->in_room;

    time = amount / 15;
    
    if (time == 0) 
        return;

    liqShort = liquidManager->find( out->value2() )->getShortDescr( );
    time = std::max( 2, time );
    pool = get_obj_room_vnum( room, OBJ_VNUM_POOL ); 
    
    if (pool) {
        /* mix two liquids */
        if (liqShort.ruscase( '1' ) != pool->getMaterial( )) {
            liqShort = "бурд|а|ы|е|у|ой|е";
        } 
        else { /* same liquid */ 
            pool->timer += time;
            pool->value0(max( 1, pool->timer / 10 ));
            act( "Лужа $n2 растекается еще шире.", ch, liqShort.c_str( ), 0, TO_ALL );
            save_items(room);
            return;
        }
    }
    else /* new pool */ {
        pool = create_object(get_obj_index(OBJ_VNUM_POOL), 0);
    }        
    
    act( "На земле образуется лужа $n2.", ch, liqShort.c_str( ), 0, TO_ALL );
    
    pool->fmtShortDescr( pool->pIndexData->short_descr, liqShort.ruscase( '2' ).c_str( ) );
    pool->fmtDescription( pool->pIndexData->description, liqShort.ruscase( '2' ).c_str( ) );
    pool->setMaterial( liqShort.ruscase( '1' ).c_str( ) );        
        
    pool->timer += time;
    pool->value0(max( 1, pool->timer / 10 ));

    if (!pool->in_room)
        obj_to_room(pool, room);
    else
        save_items(room);
}

static void pour_out( Character *ch, Object * out )
{
    int amount;
    Room *room = ch->in_room;

    if (out->value1() == 0) {
        act( "Ты переворачиваешь $o4, однако оттуда не выливается ни капли.", ch, out, 0, TO_CHAR );
        act( "Приговаривая 'ну котеночек, ну еще капельку', $c1 переворачивает и трясет $o5.", ch, out, 0, TO_ROOM );
        return;
    }
    
    amount = out->value1();
    out->value1(0);
    out->value3(0);
   
    Liquid *liq =  liquidManager->find( out->value2() );
    const char *liqname = liq->getName( ).c_str( );
    DLString liqShort = liq->getShortDescr( );

    if (oprog_empty(out, ch, liqname, amount))
	return;

    if (IS_WATER( room )) {
        ch->pecho( "Ты переворачиваешь %O4, выливая %N4 в %N4.", out, liqShort.c_str( ), room->pIndexData->liquid->getShortDescr( ).c_str( ) );
        ch->recho( "%^C1 переворачивает %O4, выливая %N4 в %N4.", ch, out, liqShort.c_str( ), room->pIndexData->liquid->getShortDescr( ).c_str( ) );
    }
    else if (room->getSectorType() == SECT_AIR) {
        act( "Ты переворачиваешь $o4, и струя $N2 устремляется вниз.", ch, out, liqShort.c_str( ), TO_CHAR );
        act( "$c1 переворачивает $o4, и струя $N2 устремляется вниз.", ch, out, liqShort.c_str( ), TO_ROOM );
    }
    else if (room->getSectorType() == SECT_DESERT) {
        act( "Ты переворачиваешь $o4, выливая $N4 на песок.", ch, out, liqShort.c_str( ), TO_CHAR );
        act( "$c1 переворачивает $o4, выливая $N4 на песок.", ch, out, liqShort.c_str( ), TO_ROOM );
        act( "Лужа $n2 с шипением испаряется.", ch, liqShort.c_str( ), 0, TO_ALL );
    }
    else {
        act( "Ты переворачиваешь $o4, выливая $N4 на землю.", ch, out, liqShort.c_str( ), TO_CHAR );
        act( "$c1 переворачивает $o4, выливая $N4 на землю.", ch, out, liqShort.c_str( ), TO_ROOM );
        create_pool( ch, out, amount );
    }

    if (out->behavior && out->behavior.getDynamicPointer<DrinkContainer>( ))
        out->behavior.getDynamicPointer<DrinkContainer>( )->pourOut( ch, amount );
}

static void oprog_pour_out( Object *obj, Character *ch, Object *out, const char *liqname, int amount )
{
    FENIA_VOID_CALL( obj, "PourOut", "COsi", ch, out, liqname, amount );
    FENIA_NDX_VOID_CALL( obj, "PourOut", "OCOsi", obj, ch, out, liqname, amount );
}

static void mprog_pour_out( Character *victim, Character *ch, Object *out, const char *liqname, int amount  )
{
    FENIA_VOID_CALL( victim, "PourOut", "COsi", ch, out, liqname, amount );
    FENIA_NDX_VOID_CALL( victim->getNPC( ), "PourOut", "CCOsi", victim, ch, out, liqname, amount );
}

static void pour_out( Character *ch, Object * out, Character *victim )
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
            victim->println( "Ты чувствуешь влагу на теле." );

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

    if (in->item_type != ITEM_DRINK_CON) {
        if (!vch || vch == ch)
            ch->pecho("Ты пытаешься налить что-то в %O4, а это не емкость для жидкости.", in);
        else
            ch->pecho("В руках у %C2 зажата совсем не емкость для жидкости.", vch);
        return;
    }

    if (drink_is_closed( in, ch ))
        return;

    if (in == out) {
        ch->pecho("Ты не можешь перелить из %1$O2 в сам%1$Gое|ого|у|их себя!", in);
        return;
    }

    if (in->value1() != 0 && in->value2() != out->value2()) {
        if (!vch || vch == ch)
            ch->pecho("В %O4 налита другая жидкость.", in);
        else
            ch->pecho("В %O4 в руках у %C2 налита другая жидкость.", in, vch);
        return;
    }

    if (out->value1() == 0) {
        act_p("В $o6 нет ничего, что можно вылить.",ch,out,0,TO_CHAR,POS_RESTING);
        return;
    }

    if (in->value1() >= in->value0()) {
        if (!vch || vch == ch)
            ch->pecho( "%1$^O1 уже полностью заполне%1$Gно|н|на|ны.", in );
        else
            ch->pecho( "%1$^O1 в руках у %2$C2 уже полностью заполне%1$Gно|н|на|ны.", in, vch );
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
        ch->send_to("Вылить что и куда?\n\r");
        return;
    }

    if ((out = get_obj_carry(ch,arg1.c_str( ))) == 0) {
        ch->println("У тебя в инвентаре нет такой емкости для жидкости.");
        return;
    }

    if (out->item_type != ITEM_DRINK_CON) {
        ch->pecho("%^O1 - не емкость для жидкости.", out);
        return;
    }

    if (drink_is_closed( out, ch ))
        return;
    
    if (arg2.empty( )) {
        pour_out( ch, out );
        return;
    }

    if (( vch = get_char_room(ch, arg2.c_str( ) ) ) == 0) {
        ch->println( "Вылить на кого?" );
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
    arg2 = arguments.getOneArgument( ); // drink2 or victim or out or empty
    arg3 = arguments.getOneArgument( ); // victim or empty

    if (arg1.empty( )) {
        ch->send_to("Вылить что и куда?\n\r");
        return;
    }

    if ((out = get_obj_carry(ch,arg1.c_str( ))) == 0) {
        ch->println("У тебя в инвентаре нет такой емкости для жидкости.");
        return;
    }

    if (out->item_type != ITEM_DRINK_CON) {
        ch->pecho("%^O1 - не емкость для жидкости.", out);
        return;
    }

    if (drink_is_closed( out, ch ))
        return;
    
    if (arg2 == "out" || arg2.empty( )) {
        if (!arg3.empty( )) {
            if (( vch = get_char_room(ch, arg3.c_str( ) ) ) == 0) {
                ch->println( "Вылить на кого?" );
                return;
            }

            pour_out( ch, out, vch );
        }
        else
            pour_out( ch, out );

        return;
    }

    if ((in = get_obj_here(ch,arg2.c_str( ))) == 0) {
        vch = get_char_room(ch,arg2.c_str( ));

        if (vch == 0) {
            ch->send_to("Вылить во что?\n\r");
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
    int amount;
    Liquid *liquid;
    DrinkContainer::Pointer drink;
    RoomIndexData *pRoom = ch->in_room->pIndexData;
    DLString arguments = constArguments, arg;
    
    arg = arguments.getOneArgument( );

    if (arg.empty( )) {
        obj = get_obj_room_type( ch, ITEM_FOUNTAIN );
        if (!obj && pRoom->liquid == liq_none) 
            if (!IS_SET(ch->in_room->room_flags, ROOM_NEAR_WATER)) {
                ch->send_to("Выпить что?\n\r");
                return;
            }
    }
    else {
        if (( obj = get_obj_here( ch, arg ) ) == 0) {
            ch->send_to("Ты не находишь это.\n\r");
            return;
        }
    }

    if (obj) {
        switch (obj->item_type) {
        default:
            ch->pecho("Ты не можешь пить из %O2.", obj);
            return;

        case ITEM_FOUNTAIN:
            liquid = liquidManager->find( obj->value2() );
            amount = liquid->getSipSize( ) * 3;
            break;

        case ITEM_DRINK_CON:
            if (drink_is_closed( obj, ch ))
                return;

            if (obj->value1() <= 0) {
                ch->send_to("Здесь пусто.\n\r");
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
        act( "$c1 пьет $T из $o2.", ch,obj,buf.c_str( ),TO_ROOM );
        act( "Ты пьешь $T из $o2.", ch,obj,buf.c_str( ),TO_CHAR );
    } else {
        act( "$c1 зачерпывает и пьет $T.", ch, 0, buf.c_str( ),TO_ROOM );
        act( "Ты зачерпываешь и пьешь $T.", ch, 0, buf.c_str( ),TO_CHAR );
    }

    if (ch->fighting != 0)
         ch->setWaitViolence( 3 );
    
    if (!ch->is_npc( )) 
        for (int i = 0; i < desireManager->size( ); i++)
            desireManager->find( i )->drink( ch->getPC( ), amount, liquid );
    
    if (obj && (IS_SET( obj->value3(), DRINK_POISONED ) || obj->isAffected(gsn_poison)))
    {
        /* The drink was poisoned ! */
        Affect af;

        act( "$c1 хватается за горло и задыхается.",ch,0,0,TO_ROOM );
        ch->send_to("Ты хватаешься за горло и задыхаешься.\n\r");
        af.bitvector.setTable(&affect_flags);
        af.type      = gsn_poison;
        af.level     = number_fuzzy(amount);
        af.duration  = 3 * amount;
        af.bitvector.setValue(AFF_POISON);
        affect_join( ch, &af );
    }

    if (obj && obj->value0() > 0)
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

    if (obj && IS_OBJ_STAT(obj, ITEM_BLESS) && immune_check(ch, DAM_HOLY, DAMF_OTHER) == RESIST_VULNERABLE) {
        ch->pecho("Святость %O2 обжигает твои внутренности!", obj);
        ch->recho("Лицо %^C2 искажается гримасой боли.", ch);
        rawdamage(ch, ch, DAM_HOLY, ch->hit / 100 + 1, true);
    }
}


/*---------------------------------------------------------------------------
 * smell affects
 *--------------------------------------------------------------------------*/
bool oprog_smell_liquid(Liquid *liq, Character *ch)
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
        
        ch->pecho( msg.c_str( ), liq->getShortDescr( ).c_str( ) );
        return true;
}        

AFFECT_DECL(PouredLiquid);
TYPE_AFFECT(bool, PouredLiquid)::smell( Character *ch, Character *victim, Affect *paf ) 
{
    bool rc = false;
    vector<int> bits = paf->global.toArray( );

    for (unsigned int i = 0; i < bits.size( ); i++) {
        Liquid *liq = liquidManager->find( bits[i] );
        if (oprog_smell_liquid(liq, victim))
            rc = true;
    }
    
    return rc;
}

VOID_AFFECT(PouredLiquid)::saves( Character *ch, Character *victim, int &save, int dam_type, Affect *paf ) 
{
    bool hasVuln = false;
    
    if (dam_type != DAM_CHARM)
        return;
    
    if (victim->getRace( ) == race_felar || victim->getRace( ) == race_cat) { 
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

