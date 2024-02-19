/* $Id: handler.cpp,v 1.1.2.31.6.19 2009/09/17 18:08:56 rufina Exp $
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

#include <string.h>

#include "logstream.h"
#include "grammar_entities_impl.h"

#include "objectmanager.h"
#include "npcharactermanager.h"
#include "pcharactermanager.h"

#include "affecthandler.h"
#include "skill.h"
#include "skillreference.h"
#include "dreamland.h"
#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "save.h"
#include "merc.h"
#include "descriptor.h"
#include "interp.h"

#include "act.h"
#include "act_move.h"
#include "mercdb.h"

#include "handler.h"
#include "def.h"

#define OBJ_VNUM_SILVER_ONE              1
#define OBJ_VNUM_GOLD_ONE              2
#define OBJ_VNUM_GOLD_SOME              3
#define OBJ_VNUM_SILVER_SOME              4
#define OBJ_VNUM_COINS                      5

GSN(fire_breath);
GSN(sand_storm);
GSN(dirt_kicking);
GSN(doppelganger);

void get_money_here( Object *list, int &gold, int &silver )
{
    Object *obj, *obj_next;

    for ( obj = list; obj != 0; obj = obj_next )
    {
        obj_next = obj->next_content;

        switch ( obj->pIndexData->vnum ) {
        case OBJ_VNUM_SILVER_ONE:
            silver += 1;
            extract_obj(obj);
            break;

        case OBJ_VNUM_GOLD_ONE:
            gold += 1;
            extract_obj( obj );
            break;

        case OBJ_VNUM_SILVER_SOME:
            silver += obj->value0();
            extract_obj(obj);
            break;

        case OBJ_VNUM_GOLD_SOME:
            gold += obj->value1();
            extract_obj( obj );
            break;

        case OBJ_VNUM_COINS:
            silver += obj->value0();
            gold += obj->value1();
            extract_obj(obj);
            break;
        }
    }
}

/*
 * Create a 'money' obj.
 */
Object *create_money( int gold, int silver )
{
    Object *obj;

    if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) )
    {
        bug( "Create_money: zero or negative money.",min(gold,silver));
        gold = max(1,gold);
        silver = max(1,silver);
    }

    if (gold == 0 && silver == 1)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
    }
    else if (gold == 1 && silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
    }
    else if (silver == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
        DLString moneyArg;
        moneyArg << gold << " золот|" << GET_COUNT( gold, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых" )
        << " монет|" << GET_COUNT(gold, "а|ы|е|у|ой|е", "ы||ам|ы|ами|ах", "||ам||ами|ах");
        obj->fmtShortDescr( moneyArg.c_str() );
        obj->value1(gold);
        obj->cost               = 100 * gold;
        obj->weight                = gold/5;
    }
    else if (gold == 0)
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
        DLString moneyArg;
        moneyArg << silver << " серебрян|" << GET_COUNT(silver, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых")
        << " монет|" << GET_COUNT(silver, "а|ы|е|у|ой|е", "ы||ам|ы|ами|ах", "||ам||ами|ах");
        obj->fmtShortDescr( moneyArg.c_str() );
        obj->value0(silver);
        obj->cost               = silver;
        obj->weight                = silver/20;
    }

    else
    {
        obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
        DLString moneyArg;
        moneyArg << gold << " золот|" << GET_COUNT( gold, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых" )
        << " и " << silver << " серебрян|" << GET_COUNT(silver, "ая|ой|ой|ую|ой|ой", "ые|ых|ым|ые|ыми|ых", "ых|ых|ым|ых|ыми|ых")
        << " монет|" << GET_COUNT(silver, "а|ы|е|у|ой|е", "ы||ам|ы|ами|ах", "||ам||ами|ах");
        obj->fmtShortDescr( moneyArg.c_str() );
        obj->value0(silver);
        obj->value1(gold);
        obj->cost                = 100 * gold + silver;
        obj->weight                = gold / 5 + silver / 20;
    }

    return obj;
}

DLString describe_money( int gold, int silver, const Grammar::Case &gcase )
{
    static const char *cases_gold [] = {
        "золот%1$Iая|ые|ых",
        "золот%1$Iая|ые|ых",
        "золот%1$Iой|ых|ых",
        "золот%1$Iой|ым|ым",
        "золот%1$Iую|ые|ых",
        "золот%1$Iой|ыми|ыми",
        "золот%1$Iой|ых|ых",
    };
    static const char *cases_silver [] = {
        "серебрян%1$Iая|ые|ых",
        "серебрян%1$Iая|ые|ых",
        "серебрян%1$Iой|ых|ых",
        "серебрян%1$Iой|ым|ым",
        "серебрян%1$Iую|ые|ых",
        "серебрян%1$Iой|ыми|ыми",
        "серебрян%1$Iой|ых|ых",
    };
    static const char *cases_coin [] = {
        "моне%1$Iта|ты|т",
        "моне%1$Iта|ты|т",
        "моне%1$Iты|т|т",
        "моне%1$Iте|там|там",
        "моне%1$Iту|ты|т",
        "моне%1$Iтой|тами|тами",
        "моне%1$Iте|тах|тах",
    };

    DLString msg;
    
    if (gold > 0)
        msg << gold << " " << fmt( 0, cases_gold[gcase], gold );

    if (silver > 0) {
        if (gold > 0)
            msg << " и ";

        msg << silver << " " << fmt( 0, cases_silver[gcase], silver);
    }
    
    msg << " " << fmt( 0, cases_coin[gcase], silver > 0 ? silver : gold );
    return msg;
}

/* 
 * returns number of people on an object 
 */
int count_users(Object *obj)
{
    Character *fch;
    int count = 0;

    if (obj->in_room == 0)
        return 0;

    for (fch = obj->in_room->people; fch != 0; fch = fch->next_in_room)
        if (fch->on == obj)
            count++;

    return count;
}


Character * find_char( Character *ch, const char *cArgument, int door, int *range, ostringstream &errbuf )
{
    char argument[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    EXIT_DATA *pExit, *bExit;
    Room *dest_room, *back_room;
    Character *target;
    int number = 0, opdoor;
    
    strcpy( argument, cArgument );
    number = number_argument(argument,arg);
    dest_room = ch->in_room;

    // Look for target in the same room only if a non-ranged lookup is requested.
    if (door == -1 && (target = get_char_room(ch,dest_room,arg,&number)) != 0)
        return target;

    opdoor = dirs[door].rev;

    while (*range > 0)
    {
        *range = *range - 1;
        /* find target room */
        back_room = dest_room;

        if ( (pExit = dest_room->exit[door]) == 0
            || !ch->can_see( pExit )
            || (dest_room = pExit->u1.to_room ) == 0
            || IS_SET(pExit->exit_info,EX_CLOSED) )
            break;

        if ( (bExit = dest_room->exit[opdoor]) == 0
            || bExit->u1.to_room != back_room)
        {
            errbuf << "Ты не сможешь добраться до них через односторонний проход." << endl;
            return 0;
        }

        if ((target = get_char_room(ch,dest_room,arg,&number)) != 0 )
            return target;
    }
    
    errbuf << "Ты не видишь " << dirs[door].where << " никого с таким именем." << endl;
    return 0;
}
        

void reboot_anatolia( void )
{
        Descriptor *d,*d_next;
        
        LogStream::sendNotice( ) << "Rebooting DREAM LAND." << endl;

        for ( d = descriptor_list; d != 0; d = d_next )
        {
                d_next = d->next;
                d->send("Мир Мечты уходит на перезагрузку ПРЯМО СЕЙЧАС!\n");

                if (d->character && d->connected == CON_PLAYING)
                        d->character->getPC( )->save();

                d->close();
        }
        dreamland->shutdown( );

        return;
}


void eyes_blinded_msg( Character *ch )
{
    if (!IS_AFFECTED(ch, AFF_BLIND))
        return;

    for (auto &paf: ch->affected.findAllWithBits(&affect_flags, AFF_BLIND)) {
        if (paf->type == gsn_fire_breath)
            ch->pecho( "Твои глаза слезятся из-за дыма, и ты ничего не видишь." );
        else if (paf->type == gsn_sand_storm)
            ch->pecho( "Песок в глазах мешает тебе что-либо разглядеть." );
        else if (paf->type == gsn_dirt_kicking)
            ch->pecho( "Ты ничего не видишь из-за пыли, попавшей в глаза." );
        else
            continue;

        return;
    }

    ch->pecho( "Твои глаза слепы, ты ничего не видишь!" );
}

/*--------------------------------------------------------------
 * character extraction
 *--------------------------------------------------------------*/
static bool char_is_nodrop( Character *ch )
{
    NPCharacter *npc = ch->getNPC();

    if (!npc)
        return false;
    
    if (IS_SET(npc->pIndexData->area->area_flag, AREA_NOSAVEDROP))
        return true;
    
    if (IS_SET(ch->act, ACT_NOSAVEDROP))
        return true;

    return false;
}

/*
 * nuke pet on extraction
 */
void nuke_pets( PCharacter *ch, int flags )
{
    NPCharacter *pet = ch->pet;
    
    if (!pet)
        return;

    follower_stop(pet);
   
    
    
    if (IS_SET(flags, FEXTRACT_TOTAL))
        extract_char( pet, IS_SET(flags, FEXTRACT_COUNT));
    else
        pet->setDead( );

    ch->pet = NULL;
}

/*
 * Оповестить о extract-е всех, кто на нас ссылался
 */
void notify_referers( Character *ch, int flags )
{
    Character *wch;
    AffectSource chSource(ch);

    for (wch = char_list; wch != 0; wch = wch->next) {
        if (IS_SET(flags, FEXTRACT_TOTAL) && wch->reply == ch)
            wch->reply = 0;

        if (wch->doppel == ch && wch->isAffected(gsn_doppelganger)) {
            wch->pecho("Ты принимаешь свое истинное обличье.");
            
            // TODO rework with verbose affect strip
            if (gsn_doppelganger->getAffect( ))
                gsn_doppelganger->getAffect( )->onRemove(SpellTarget::Pointer(NEW, wch), 0);

            affect_strip(wch,gsn_doppelganger);
        }

        // All affects on others that ch created are no longer associated with this char.
        for (auto &paf: wch->affected)
            paf->sources.remove(chSource);

        if (wch->is_npc( ) && wch->getNPC( )->behavior) 
            wch->getNPC( )->behavior->extractNotify( ch, IS_SET(flags, FEXTRACT_TOTAL), IS_SET(flags, FEXTRACT_COUNT) );
        
        if (IS_SET(flags, FEXTRACT_TOTAL|FEXTRACT_LASTFOUGHT) && wch->last_fought == ch)
            wch->last_fought = 0;
    }
    
    // All room affects that ch created are no longer associated with this char,
    // meaning no damage will be done on behalf of ch after their death.
    for (auto &room: roomAffected)
        for (auto &paf: room->affected)
            paf->sources.remove(chSource);

    guarding_clear( ch );
}

/*
 * Extract a char from the world.
 */
void extract_char( Character *ch, bool count )
{
    Object *obj;
    Object *obj_next;
    int flags;

    if (ch->extracted)  {
        LogStream::sendError( ) << "Warning! Extraction of " << ch->getNameC() << endl;
        return; 
    }
    else
        ch->extracted = true;  

    NPCharacter *npc = ch->getNPC( );
    PCharacter *pc = ch->getPC( );
    
    if (char_is_nodrop( ch ))
        count = true;

    flags = FEXTRACT_TOTAL | (count ? FEXTRACT_COUNT : 0);

    if (!npc)
        nuke_pets( pc, flags );
        
    follower_die(ch);

    stop_fighting( ch, true );

    for (obj = ch->carrying; obj != 0; obj = obj_next) {
        obj_next = obj->next_content;
        extract_obj_1( obj, count );
    }
    
    undig( ch );
    ch->dismount( );
    
    notify_referers( ch, flags );

    if (npc && npc->switchedFrom)
        interpret_raw( ch, "return" );

    char_from_room( ch );

    if (npc) {
        if (count)
            --npc->pIndexData->count;
    }

    /*paranoid*/
    if (!npc && pc->switchedTo && pc->switchedTo->desc) {
        pc->switchedTo->desc->close( );
        pc->switchedTo->switchedFrom = 0;
        pc->switchedTo = 0;
        LogStream::sendError() 
            << "attempt to extract original PCwhile in switch: " 
            << pc->getNameC() << endl;
    }

    char_from_list( ch, &char_list );
    
    if (ch->desc)
        ch->desc->character = 0;
        
    mprog_extract( ch, count );
    
    if (!npc)
        PCharacterManager::extract( pc );
    else
        NPCharacterManager::extract( npc );
}

DLString quality_percent( int c )
{
    DLString str;

    if (c >  99) return "{Cотличн|ое{x|ого{x|ому{x|ое{x|ым{x|ом{x";
    if (c >= 80) return "{cхорош|ее{x|его{x|ему{x|ее{x|им{x|ем{x";
    if (c >= 60) return "{Yнормальн|ое{x|ого{x|ому{x|ое{x|ым{x|ом{x";
    if (c >= 40) return "{yсредн|ее{x|его{x|ему{x|ее{x|им{x|ем{x";
    if (c >= 20) return "{Rплох|ое{x|ого{x|ому{x|ое{x|им{x|ом{x";
    return "{rужасн|ое{x|ого{x|ому{x|ое{x|ым{x|ом{x";
}


