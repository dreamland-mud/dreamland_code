/* $Id$
 *
 * ruffina, 2004
 */
#include "drink_utils.h"
#include "drink_commands.h"
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
#include "object.h"
#include "affect.h"
#include "skillreference.h"

#include "loadsave.h"
#include "damageflags.h"
#include "save.h"
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

WEARLOC(hold);
PROF(vampire);
GSN(poison);
GSN(poured_liquid);
RACE(felar);
RACE(cat);
RACE(satyr);
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

    if (arg1.empty( )) {
        fountain = get_obj_room_type( ch, ITEM_FOUNTAIN );
    }
    else {
        if (( fountain = get_obj_here( ch, arg1.c_str( ) ) ))
            if (fountain->item_type != ITEM_FOUNTAIN)
                fountain = 0;
    }

    if (!fountain) {
        ch->send_to("Здесь нет источника!\n\r");
        return;
    }

    if (obj->item_type != ITEM_DRINK_CON) {
        ch->pecho( "Ты не можешь наполнить %O4.", obj );
        return;
    }

    if (drink_is_closed( obj, ch ))
        return;

    if (obj->value[1] != 0 && obj->value[2] != fountain->value[2]) {
        ch->pecho("В %O4 налита другая жидкость.", obj);
        return;
    }

    if (obj->value[1] >= obj->value[0]) {
        ch->pecho("%1$^O1 уже наполн%1$Gено|ен|на|ны до краев.", obj);
        return;
    }
    
    liq = liquidManager->find( fountain->value[2] );
    ch->pecho( "Ты наполняешь %O4 %N5 из %O2.",
               obj, liq->getShortDescr( ).c_str( ), fountain );
    ch->recho( "%^C1 наполняет %O4 %N5 из %O2.",
               ch, obj, liq->getShortDescr( ).c_str( ), fountain );

    amount = obj->value[0] - obj->value[1];
    obj->value[2] = fountain->value[2];
    obj->value[1] = obj->value[0];

    if (obj->behavior && ( drink = obj->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->fill( ch, fountain, amount );
}

/*
 * 'pour' command
 */
void CPour::createPool( Character *ch, Object *out, int amount ) 
{
    Object *pool;
    int time;
    DLString liqShort;
    Room *room = ch->in_room;

    time = amount / 15;
    
    if (time == 0) 
        return;

    liqShort = liquidManager->find( out->value[2] )->getShortDescr( );
    time = std::max( 2, time );
    pool = get_obj_room_vnum( room, OBJ_VNUM_POOL ); 
    
    if (pool) {
        /* mix two liquids */
        if (liqShort.ruscase( '1' ) != pool->getMaterial( )) {
            liqShort = "бурд|а|ы|е|у|ой|е";
        } 
        else { /* same liquid */ 
            pool->timer += time;
            pool->value[0] = max( 1, pool->timer / 10 );
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
    pool->value[0] = max( 1, pool->timer / 10 );

    if (!pool->in_room)
        obj_to_room(pool, room);
    else
        save_items(room);
}

void CPour::pourOut( Character *ch, Object * out )
{
    int amount;
    DLString liqShort;
    Room *room = ch->in_room;

    if (out->value[1] == 0) {
        act( "Ты переворачиваешь $o4, однако оттуда не выливается ни капли.", ch, out, 0, TO_CHAR );
        act( "Приговаривая 'ну котеночек, ну еще капельку', $c1 переворачивает и трясет $o5.", ch, out, 0, TO_ROOM );
        return;
    }
    
    amount = out->value[1];
    out->value[1] = 0;
    out->value[3] = 0;
    
    liqShort = liquidManager->find( out->value[2] )->getShortDescr( );

    if (IS_WATER( room )) {
        ch->pecho( "Ты переворачиваешь %O4, выливая %N4 в %N4.", out, liqShort.c_str( ), room->liquid->getShortDescr( ).c_str( ) );
        ch->recho( "%^C1 переворачивает %O4, выливая %N4 в %N4.", ch, out, liqShort.c_str( ), room->liquid->getShortDescr( ).c_str( ) );
    }
    else if (room->sector_type == SECT_AIR) {
        act( "Ты переворачиваешь $o4, и струя $N2 устремляется вниз.", ch, out, liqShort.c_str( ), TO_CHAR );
        act( "$c1 переворачивает $o4, и струя $N2 устремляется вниз.", ch, out, liqShort.c_str( ), TO_ROOM );
    }
    else if (room->sector_type == SECT_DESERT) {
        act( "Ты переворачиваешь $o4, выливая $N4 на песок.", ch, out, liqShort.c_str( ), TO_CHAR );
        act( "$c1 переворачивает $o4, выливая $N4 на песок.", ch, out, liqShort.c_str( ), TO_ROOM );
        act( "Лужа $n2 с шипением испаряется.", ch, liqShort.c_str( ), 0, TO_ALL );
    }
    else {
        act( "Ты переворачиваешь $o4, выливая $N4 на землю.", ch, out, liqShort.c_str( ), TO_CHAR );
        act( "$c1 переворачивает $o4, выливая $N4 на землю.", ch, out, liqShort.c_str( ), TO_ROOM );
        createPool( ch, out, amount );
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

void CPour::pourOut( Character *ch, Object * out, Character *victim )
{
    Liquid *liquid;
    int sips, amount;
    DLString msgRoom, msgVict, msgChar;
    DLString msgOther, msgSelf;

    liquid = liquidManager->find( out->value[2] );
    amount = out->value[1];
    sips = max( 1, amount / liquid->getSipSize( ) );

    if (out->value[1] == 0) {
        msgChar = "Ты опрокидываешь на %2$C4 %3$O4, однако оттуда не выливается ни капли.";
        msgVict = "%1$C1 переворачивает над тобой %3$O4, однако оттуда не выливается ни капли.";
        msgRoom = "%1$C1 переворачивает над %2$C5 %3$O4, однако оттуда не выливается ни капли.";

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
    
    if (out->value[1] == 0)
        return;

    out->value[1] = 0;
    out->value[3] = 0;
    
    if (sips >= 5) {
        if (liq_water == liquid) {
            Affect *paf, *paf_next;
            bool rc = false;

            for (paf = victim->affected; paf; paf = paf_next) {
                paf_next = paf->next;

                if (paf->type == gsn_poured_liquid) {
                    paf->duration -= sips / 5;
                    rc = true;

                    if (paf->duration < 0) 
                        affect_remove( victim, paf );
                }
            }

            if (rc) 
                victim->pecho( POS_RESTING, "Вода смывает с тебя часть посторонних запахов." );
        }
        else {
            Affect af;
            
            af.type = gsn_poured_liquid;
            af.where = TO_LIQUIDS;
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

COMMAND( CPour, "pour" )
{
    DLString arg1, arg2, arg3;
    DLString arguments = constArguments;
    Object *out, *in;
    Character *vch = 0;
    int amount;
    DrinkContainer::Pointer drink;
    Liquid *liq;
    
    arg1 = arguments.getOneArgument( ); // drink1
    arg2 = arguments.getOneArgument( ); // drink2 or victim or out or empty
    arg3 = arguments.getOneArgument( ); // victim or empty

    if (arg1.empty( )) {
        ch->send_to("Вылить что и куда?\n\r");
        return;
    }

    if ((out = get_obj_carry(ch,arg1.c_str( ))) == 0) {
        ch->send_to("У тебя нет этого.\n\r");
        return;
    }

    if (out->item_type != ITEM_DRINK_CON) {
        ch->pecho("%^O1 - не емкость для жидкости.", out);
        return;
    }

    if (drink_is_closed( out, ch ))
        return;
    
    liq = liquidManager->find( out->value[2] );
    
    if (arg2 == "out" || arg2.empty( )) {
        if (!arg3.empty( )) {
            if (( vch = get_char_room(ch, arg3.c_str( ) ) ) == 0) {
                ch->println( "Вылить на кого?" );
                return;
            }

            pourOut( ch, out, vch );
        }
        else
            pourOut( ch, out );

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
            ch->println("Во что?");
            return;
        }
    }

    if (in->item_type != ITEM_DRINK_CON) {
        ch->send_to("Ты можешь вылить только в другую емкость для жидкости.\n\r");
        return;
    }

    if (drink_is_closed( in, ch ))
        return;

    if (in == out) {
        ch->send_to("Ты не можешь изменить законы физики!\n\r");
        return;
    }

    if (in->value[1] != 0 && in->value[2] != out->value[2]) {
        ch->pecho("В %O4 налита другая жидкость.", in);
        return;
    }

    if (out->value[1] == 0) {
        act_p("В $o6 нет ничего, что можно вылить.",ch,out,0,TO_CHAR,POS_RESTING);
        return;
    }

    if (in->value[1] >= in->value[0]) {
        ch->pecho( "%1$^O1 уже полностью заполне%1$Gно|н|на|ны.", in );
        return;
    }

    amount = min(out->value[1],in->value[0] - in->value[1]);

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];

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

/*
 * 'drink' command
 */
static bool mprog_drink( Character *ch, Object *obj, const char *liq, int amount )
{
    FENIA_CALL(ch, "Drink", "Osi", obj, liq, amount);
    FENIA_NDX_CALL(ch->getNPC( ), "Drink", "COsi", ch, obj, liq, amount);
    return false;
}

static bool oprog_drink( Object *obj, Character *ch, const char *liq, int amount )
{
    FENIA_CALL( obj, "Drink", "Csi", ch, liq, amount );
    FENIA_NDX_CALL( obj, "Drink", "OCsi", obj, ch, liq, amount );
    return false;
}


CMDRUN( drink )
{
    Object *obj;
    int amount;
    Liquid *liquid;
    DrinkContainer::Pointer drink;
    DLString arguments = constArguments, arg;

    arg = arguments.getOneArgument( );

    if (arg.empty( )) {
        if (!( obj = get_obj_room_type( ch, ITEM_FOUNTAIN ) )) {
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

    switch (obj->item_type) {
    default:
        ch->pecho("Ты не можешь пить из %O2.", obj);
        return;

    case ITEM_FOUNTAIN:
        liquid = liquidManager->find( obj->value[2] );
        amount = liquid->getSipSize( ) * 3;
        break;

    case ITEM_DRINK_CON:
        if (drink_is_closed( obj, ch ))
            return;

        if (obj->value[1] <= 0) {
            ch->send_to("Здесь пусто.\n\r");
            return;
        }

        liquid = liquidManager->find( obj->value[2] );
        amount = liquid->getSipSize( );
        amount = min(amount, obj->value[1]);
        break;
    }
    
    if (!ch->is_npc( ))
        for (int i = 0; i < desireManager->size( ); i++)
            if (!desireManager->find( i )->canDrink( ch->getPC( ) ))
                return;
    
    DLString buf = liquid->getShortDescr( ).ruscase( '4' );

    act( "$c1 пьет $T из $o2.", ch,obj,buf.c_str( ),TO_ROOM );
    act( "Ты пьешь $T из $o2.", ch,obj,buf.c_str( ),TO_CHAR );

    if (ch->fighting != 0)
         ch->setWaitViolence( 3 );
    
    if (!ch->is_npc( )) 
        for (int i = 0; i < desireManager->size( ); i++)
            desireManager->find( i )->drink( ch->getPC( ), amount, liquid );
    
    if (IS_SET( obj->value[3], DRINK_POISONED ))
    {
        /* The drink was poisoned ! */
        Affect af;

        act( "$c1 хватается за горло и задыхается.",ch,0,0,TO_ROOM );
        ch->send_to("Ты хватаешься за горло и задыхаешься.\n\r");
        af.where     = TO_AFFECTS;
        af.type      = gsn_poison;
        af.level     = number_fuzzy(amount);
        af.duration  = 3 * amount;
        af.bitvector = AFF_POISON;
        affect_join( ch, &af );
    }
    
    if (obj->value[0] > 0)
        obj->value[1] -= amount;

    if (obj->behavior && ( drink = obj->behavior.getDynamicPointer<DrinkContainer>( ) ))
        drink->drink( ch, amount );

    if (mprog_drink( ch, obj, liquid->getName( ).c_str( ), amount ))
        return;

    if (oprog_drink( obj, ch, liquid->getName( ).c_str( ), amount ))
        return;
}


/*---------------------------------------------------------------------------
 * smell affects
 *--------------------------------------------------------------------------*/
AFFECT_DECL(PouredLiquid);
TYPE_AFFECT(bool, PouredLiquid)::smell( Character *ch, Character *victim, Affect *paf ) 
{
    bool rc = false;
    vector<int> bits = paf->global.toArray( );

    for (unsigned int i = 0; i < bits.size( ); i++) {
        Liquid *liq = liquidManager->find( bits[i] );
        DLString msg;

        if (liq->getFlags( ).isSet( LIQF_LIQUOR ))
            msg = "Тебе в нос ударяет резкий запах %1$N2.";
        else if (liq->getFlags( ).isSet( LIQF_WINE ))
            msg = "Ты улавливаешь аромат %1$N2.";
        else if (liq->getFlags( ).isSet( LIQF_BEER ))
            msg = "Пахнет %1$N5.";
        else
            msg = "Ты чувствуешь запах %1$N2.";
        
        victim->pecho( msg.c_str( ), liq->getShortDescr( ).c_str( ) );
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

