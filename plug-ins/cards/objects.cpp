/* $Id: objects.cpp,v 1.1.2.17.6.4 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
 */

#include "objects.h"
#include "xmlattributecards.h"

#include "class.h"
#include "char.h"

#include "skill.h"
#include "skillreference.h"

#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "act_move.h"
#include "magic.h"
#include "fight.h"
#include "handler.h"
#include "act.h"
#include "mercdb.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"

GSN(fetch_card);
GSN(peek_card);
GSN(enter_card);
GSN(ace_in_sleeves);
GSN(card_vision);
GSN(pull_card);

GSN(heal);
GSN(superior_heal);
GSN(master_healing);

/*--------------------------------------------------------------------------
 * CardPackBehavior 
 *--------------------------------------------------------------------------*/

CardPackBehavior::CardPackBehavior( ) : throws( 36 )
{
}

bool CardPackBehavior::examine( Character *looker ) 
{
    Descriptor *d;
    bool fEmpty = true;
    
    if (looker->is_npc( ))
        return false;
    
    looker->printf( "Сегодня козырь %s.\r\n", 
            russian_case( XMLAttributeCards::getTrumpFace( ).mlt, 1 ).c_str( ) );

    for (d = descriptor_list; d != 0; d = d->next) {
        PCharacter *victim;
        XMLAttributeCards::Pointer attr;
        
        if (d->connected != CON_PLAYING || !d->character)
            continue;
        
        victim = d->character->getPC( );

        if (!looker->can_see( victim ))
            continue;

        attr = victim->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );
        
        if (!attr || attr->getLevel( ) < 0)
            continue;
        
        if (fEmpty)
            looker->printf( "Колода содержит: \r\n" );
        
        fEmpty = false;
        looker->printf( "    %-20s %s\r\n", 
                victim->getNameP( ),
                attr->getFace( '1' ).c_str( ) );
    }
    
    if (fEmpty) 
        looker->pecho("Похоже, сейчас твоя колода пуста.");

    return true;
}


/*
 * 'fetch card' skill
 */
bool CardPackBehavior::use( Character *user, const char *args ) 
{ 
    PCharacter *pch, *victim;
    XMLAttributeCards::Pointer myAttr;
    int chance;

    if (!gsn_fetch_card->usable( user ))
        return false;

    pch = user->getPC( );
    myAttr = pch->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );
    
    if (!myAttr)
        return false;
    
    if (args[0] == '\0') {
        pch->pecho("Какую именно карту ты хочешь вытащить из колоды?");
        return true;
    }
    
    victim = get_player_world( pch, args );
    
    if (!victim || !victim->getAttributes( ).isAvailable( "cards" )) {
        pch->pecho("Ты сейчас не видишь в колоде такой карты.");
        return true;
    }
    
    chance = gsn_fetch_card->getEffective( pch );

    if (number_percent( ) >= chance) {
        oldact("Ты пытаешься вытащить из колоды карту, но она выпадает у тебя из рук.", pch, 0, 0, TO_CHAR);
        oldact("$c1 пытается вытащить из колоды карту, но она выпадает у $x из рук.", pch, 0, 0, TO_ROOM);
        gsn_fetch_card->improve( pch, false );
    }
    else {
        OBJ_INDEX_DATA *pCardIndex;
        Object *card;

        if (!( pCardIndex = get_obj_index( OBJ_VNUM_CARD ) )) 
            return true;
    
        card = create_object( pCardIndex, 0 );
        card->timer = (1 + myAttr->getLevel( )) * 3 * 60 * 60;
        card->fmtShortDescr( card->getShortDescr( ), victim->getNameP( '2' ).c_str( ) );
        card->fmtName( card->getName( ), victim->getName( ).c_str( ) );
        
        CardBehavior::Pointer bhv( NEW );
        bhv->setObj( card );
        bhv->setPlayerName( victim->getName( ) );
        bhv->setQuality( chance );
        card->behavior.setPointer( *bhv );
        
        obj_to_char( card, pch );
        oldact("Ты перетасовываешь $o4 и сдаешь $O4.", pch, obj, card, TO_CHAR);
        oldact("$c1 перетасовывает $o4.", pch, obj, 0, TO_ROOM);
        
        gsn_fetch_card->improve( pch, true );
    }
    
    throws--;

    if (throws <= 0) {
        oldact("Это была последняя карта в $o6.", pch, obj, 0, TO_CHAR);
        extract_obj( obj );
    }
    
    return true; 
}

bool CardPackBehavior::hasTrigger( const DLString &t )
{
    return (t == "use" || t == "examine");
}



/*--------------------------------------------------------------------------
 * CardBehavior
 *--------------------------------------------------------------------------*/
CardBehavior::CardBehavior( ) 
{
}

bool CardBehavior::hasTrigger( const DLString &t )
{
    return (t == "use" || t == "examine" || t == "shake");
}

/*
 * 'enter card' skill
 */
bool CardBehavior::use( Character *user, const char *cArgs ) 
{ 
    Room *room;
    int chance;
    bool fSuccess;
    PCharacter *pch, *victim;
    DLString args = cArgs;

    if (!gsn_enter_card->usable( user ))
        return false;

    pch = user->getPC( );
    
    if (getPlayerName( ).empty( )) {
        pch->pecho("Эта карта пуста.");
        return true;
    }
   
    if (pch->fighting) {
        pch->pecho("Ты не можешь полностью сосредоточиться на карте.");
        return true;
    }
    
    pch->setWait( gsn_enter_card->getBeats(pch) );
    victim = get_player_world( pch, getPlayerName( ).c_str( ) );
    
    if (!victim 
            || !pch->can_see( victim ) 
            || !victim->getAttributes( ).isAvailable( "cards" )) 
    {
        oldact("$o1 выглядит мертвой.", pch, obj, 0, TO_CHAR);
        return true;
    }

    if (victim == pch) {
        pch->pecho("Так сильно по себе скучаешь?");
        return true;
    }

    if (is_safe_nomessage( pch, victim ) 
            || !victim->in_room->isCommon()
            || !pch->can_see( victim->in_room )) 
    {
        oldact("$o1 выглядит живой, но тебе не удается понять, как сквозь нее пройти.", pch, obj, 0, TO_CHAR);
        return true;
    }
    
    chance = quality * gsn_enter_card->getEffective( pch ) / 100;
    chance = URANGE( 5, chance, 95 );

    if (number_percent( ) >= chance) {
        fSuccess = false;

        if (number_percent( ) <= 5) 
            room = findHorribleRoom( pch );
        else
            room = get_random_room( pch );
    }
    else {
        room = victim->in_room;
        fSuccess = true;
    }

    transfer_char( pch, pch, room,
                "%1$^C1 пристально смотрит на карту.. Контуры %1$P2 тела расплываются и %1$P1 исчезает.",
                "{cТы всматриваешься в карту, постепенно замечая все новые и новые детали.{x",
                "Внезапно возникшее перед тобой плоское изображение %1$C2 постепенно обретает объем.");
                
    if (pch->pet)
        transfer_char( pch->pet, pch, room );
    
    gsn_enter_card->improve( pch, fSuccess );
    extract_obj( obj );
    return true;
}

/*
 * 'pull card' skill
 */
bool CardBehavior::command( Character *actor, const DLString &cmdName, const DLString &cmdArgs )
{
    XMLAttributeCards::Pointer hisAttr, myAttr;
    CardBehavior::Pointer myCard;
    PCharacter *pch, *victim; 
    int mana;
    
    if (actor != obj->carried_by)
        return false;
    
    if (cmdName != "shake" && cmdName != "пожать")
        return false;
    
    if (!is_name( cmdArgs.c_str( ), getPlayerName( ).c_str( ) ))
        return false;

    if (actor->is_npc( ))
        return false;

    pch = actor->getPC( );
    victim = get_player_world( pch, getPlayerName( ).c_str( ) );

    if (pch->fighting) {
        pch->pecho("Ты не можешь полностью сосредоточиться на карте.");
        return true;
    }

    if (!victim || !pch->can_see( victim )) {
        oldact("$o1 выглядит мертвой.", pch, obj, 0, TO_CHAR);
        return true;
    }

    if (pch == victim) {
        pch->pecho("Ты хватаешь себя за руку.");
        return true;
    }

    mana = gsn_pull_card->getMana(pch);

    hisAttr = victim->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );
    myAttr = pch->getAttributes( ).getAttr<XMLAttributeCards>( "cards" );
    myCard = findMyCard( pch, victim );

    pch->setWait( gsn_pull_card->getBeats(pch) );

    if (hisAttr 
        && hisAttr->getContactName( ) == pch->getName( )
        && myCard
        && gsn_pull_card->usable( pch ))
    {
        int chance;
        bool fSuccess;

        if (is_safe_nomessage( pch, victim )
            || victim->fighting
            || victim->position < POS_RESTING
            || !gsn_pull_card->usable( pch )) 
        {
            oldact("$o1 выглядит живой, но тебе не удается дотронуться до $S руки.", pch, obj, victim, TO_CHAR);
            return true;
        }

    
        if (pch->mana < mana) {
            pch->pecho("У тебя недостаточно энергии для этого.");
            return true;
        }
        else
            pch->mana -= mana;

        chance = quality * gsn_pull_card->getEffective( pch ) / 100;
        chance = chance * myCard->getQuality( ) / 100;
        chance = URANGE( 5, chance, 95 );

        if (number_percent( ) >= chance) {
            fSuccess = false;

            if (!IS_AFFECTED(victim, AFF_BLIND))
                oldact("Тебе показалось, что картинка на $o6 пошевелилась.", 
                     victim, myCard->getObj( ), 0, TO_CHAR);

            oldact("$c1 не смо$gгло|г|гла пожать твою руку.", pch, 0, victim, TO_VICT);
            
            if (number_range(1, 100) < 10) {
                oldact("{cТы пытаешься затащить $C4 к себе через карту, но вместо этого тащишь кого-то другого!{x",
                    pch, 0, victim, TO_CHAR);        
                oldact("$c1 хватает кого-то за руку и тащит к себе.. упс..", pch, 0, 0, TO_ROOM);

                multi_hit( findHorribleVictim( pch ), pch );
            }
            else {
                oldact("Твоя попытка затащить $C4 к себе провалилась.", pch, 0, victim, TO_CHAR);
            }
        }
        else {
            fSuccess = true;
            
            oldact("Ты пожимаешь протянутую тебе руку.", pch, 0, victim, TO_CHAR);
            oldact("$c1 пожимает чью-то руку.", pch, 0, 0, TO_ROOM);
            oldact("{c$o1 оживает. $C1 пожимает твою руку.\r\n{x", victim, myCard->getObj( ), pch, TO_CHAR);

            transfer_char( victim, pch, pch->in_room,
                           "%1$^C1 сплющивается до размеров игральной карты и исчезает.",
                           NULL,
                           "Внезапно возникшее перед тобой плоское изображение %1$C2 постепенно обретает объем.");

            if (victim->pet)
                transfer_char( victim->pet, pch, pch->in_room );
        }
        
        hisAttr->setContactName( "" );
        gsn_pull_card->improve( pch, fSuccess );
        extract_obj( myCard->getObj( ) );
        extract_obj( obj );
    }
    else {
        if (!hisAttr || !myCard || !gsn_pull_card->usable( victim )) 
            oldact("$C1 не сможет затащить тебя к себе через карту.", pch, 0, victim, TO_CHAR);
        else {
            if (pch->mana < mana) {
                pch->pecho("У тебя недостаточно энергии для этого.");
                return true;
            }
            else
                pch->mana -= mana;

            myAttr->setContactName( victim->getName( ) );
            oldact("Ты дотрагиваешься до $o2, мысленно протягивая $M руку.", pch, obj, victim, TO_CHAR);
            oldact("$c1 дотрагивается до $o2.", pch, obj, 0, TO_ROOM);
            oldact("{cИзображение $C2 на карте оживает и протягивает тебе руку.{x", victim, 0, pch, TO_CHAR);
        }
    }
    
    return true;
}


/*
 * 'peek card' and 'card vision' skills
 */
bool CardBehavior::examine( Character *looker ) 
{ 
    PCharacter *pch, *victim;
    Room *room;
    int chance, mana;

    if (!gsn_peek_card->usable( looker ))
        return false;

    pch = looker->getPC( );
    
    if (getPlayerName( ).empty( )) {
        pch->pecho("Эта карта пуста.");
        return true;
    }
    
    mana = gsn_peek_card->getMana(pch);
    
    if (pch->mana < mana) {
        pch->pecho("У тебя недостаточно энергии для этого.");
        return true;
    }
    
    pch->setWait(gsn_peek_card->getBeats(pch) );
        
    victim = get_player_world( pch, getPlayerName( ).c_str( ) );
   
    if (!victim 
            || !pch->can_see( victim ) 
            || !victim->getAttributes( ).isAvailable( "cards" )) 
    {
        oldact("$o1 выглядит мертвой.", pch, obj, 0, TO_CHAR);
        return true;
    }

    if (is_safe_nomessage( pch, victim ) 
            || !victim->in_room
            || !pch->can_see( victim->in_room )) 
    {
        oldact("$o1 выглядит живой, но ты не можешь сосредоточиться на мелких деталях.", pch, obj, 0, TO_CHAR);
        return true;
    }

    chance = quality * quality * gsn_peek_card->getEffective( pch ) / 10000;
    chance = URANGE( 5, chance, 95 );

    if (number_percent( ) >= chance) 
        room = get_random_room( pch );
    else
        room = victim->in_room;

    pch->printf("По деталям заднего плана ты начинаешь узнавать место. Кажется, это '%s'.\r\n", room->getName() );
    gsn_peek_card->improve( pch, true );
    pch->mana -= mana;

    if (gsn_card_vision->getLearned( looker ) > 1) {
        ostringstream buf;
        
        buf << "Тебе удается разглядеть некоторые фрагменты пейзажа более подробно:" 
            << endl
            << spoilDescription( pch, 
                                 room->getDescription(),
                                 3 * gsn_card_vision->getEffective( pch ) / 4 );
        pch->send_to( buf );

        gsn_card_vision->improve( pch, true );
    }

    return true; 
}


/*
 * 'card vision' skill
 */
DLString CardBehavior::extraDescription( Character *ch, const DLString &args )
{
    ostringstream buf;
    DLString strQuality;
    int chance;
    PCharacter *victPlayer;
    PCMemoryInterface *victMemory;
    
    if (ch->is_npc( ) 
            || !is_name( args.c_str( ), obj->getName( ) )
            || gsn_card_vision->getLearned( ch ) <= 1)
        return DLString::emptyString;

    victPlayer = PCharacterManager::findPlayer( getPlayerName( ) );
    victMemory = PCharacterManager::find( getPlayerName( ) );

    if (!victMemory)
        return DLString::emptyString;

    chance = gsn_card_vision->getEffective( ch );
    
    if (chance > 50) {
        if (quality == 100)
            strQuality = "мастерское ";
        else if (quality > 90)
            strQuality = "очень качественное ";
        else if (quality > 80)
            strQuality = "довольно четкое ";
        else if (quality > 70)
            strQuality = "не очень четкое ";
        else if (quality > 50)
            strQuality = "небрежно нарисованное ";
        else
            strQuality = "отвратительно прорисованное ";
    }
    
    buf << "На карте ты видишь " 
        << strQuality << "изображение " 
        << getPlayerName( ) << ". ";

    if (victPlayer || chance > 90) {
        XMLAttributeCards::Pointer victAttr;

        victAttr = victMemory->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );
        if (victAttr)
            buf << "Это " << victAttr->getFace( '1' ) << ".";
        else
            buf << "Это карта неизвестной масти и достоинства.";
    }

    return buf.str( );
}


/*
 * utils
 */
CardBehavior::Pointer CardBehavior::findMyCard( PCharacter *pch, PCharacter *victim )
{
    Object *o;

    for (o = victim->carrying; o; o = o->next_content)
        if (o->behavior) {
            CardBehavior::Pointer card= o->behavior.getDynamicPointer<CardBehavior>( );

            if (card && card->getPlayerName( ) == pch->getName( ))
                return card;
        }

    return CardBehavior::Pointer( );
}
    
DLString CardBehavior::spoilDescription( PCharacter *pch, const char *cText, int chance0 ) 
{
    DLString text( cText ), result;
    
    text.colourStrip( );

    for (const char *t = text.c_str( ); *t; t++) {
        if (!ispunct( *t ) 
            && !dl_isspace( *t ) 
            && !chance( chance0 ))
        {
            result.append( '.' );
        } else
            result.append( *t );
    }

    return result;
}

NPCharacter * CardBehavior::findHorribleVictim( PCharacter *pch )
{
    static const int badMobiles [] = {21811, 21806, 13233, 10031};
    static const int size = sizeof(badMobiles) / sizeof(*badMobiles);
    NPCharacter *mob;

    mob = create_mobile( get_mob_index( badMobiles[number_range( 0, size - 1 )] ) );
    char_to_room( mob, pch->in_room );
    return mob;
}

Room * CardBehavior::findHorribleRoom( PCharacter *pch )
{
    static const int badRooms [] = {32724};
    static const int size = sizeof(badRooms) / sizeof(*badRooms);
    Room *room;
    
    room = get_room_instance( badRooms[number_range( 0, size - 1 )] );
    
    return (room ? room : get_random_room( pch ));
}

/*--------------------------------------------------------------------------
 * CardSleevesBehavior 
 *--------------------------------------------------------------------------*/
CardSleevesBehavior::CardSleevesBehavior( ) 
{
}

/*
 * 'aces in sleeves' skill
 */
bool CardSleevesBehavior::canEquip( Character *ch ) 
{
    if (!gsn_ace_in_sleeves->usable( ch )) {
        oldact("Ты пытаешься напялить $o4, но шулер из тебя никакущий..", ch, obj, 0, TO_CHAR);
        oldact("$c1 пытается напялить $o4, но шулер из $x никакущий..", ch, obj, 0, TO_ROOM);
        oldact("$o1 падают на землю.", ch, obj, 0, TO_ALL);
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;
}

void CardSleevesBehavior::fight( Character *ch ) 
{
    static const int plushki [] = {
        gsn_heal,
        gsn_heal,
        gsn_superior_heal,
        gsn_master_healing,
    };
    static const int size = sizeof(plushki) / sizeof(*plushki);

    if (obj->wear_loc == wear_none)
        return;

    if (number_bits(3))
        return;

    if (number_percent( ) >= gsn_ace_in_sleeves->getEffective( ch )) {
        oldact("{cТы пытаешься вынуть туза из $o2, но роняешь его! Упс..{x", ch, obj, 0, TO_CHAR);
        oldact("{cИз $o2 $c2 выпадает туз! Шулер!{x", ch, obj, 0, TO_ROOM);
        gsn_ace_in_sleeves->improve( ch, false );
        ch->setWait(gsn_ace_in_sleeves->getBeats(ch));
    }
    else {
        oldact("{cТы достаешь из $o2 припрятанного туза.{x", ch, obj, 0, TO_CHAR);
        gsn_ace_in_sleeves->improve( ch, true );
        
        spell( plushki[number_range( 0, size - 1 )], 
                ch->getModifyLevel( ), ch, ch, true );
    }
}

