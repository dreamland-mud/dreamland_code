/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *        
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#include "logstream.h"
#include "act_move.h"
#include "portalmovement.h"
#include "commandtemplate.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"

#include "gsn_plugin.h"
#include "act.h"
#include "../anatolia/handler.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

/* command procedures needed */
void do_stand( Character *, const char * );
void do_visible( Character * );

/*-----------------------------------------------------------------------------
 * direction commands 
 *----------------------------------------------------------------------------*/
CMDRUNP( north )
{
    move_char( ch, DIR_NORTH, argument );
}

CMDRUNP( east )
{
    move_char( ch, DIR_EAST, argument );
}

CMDRUNP( south )
{
    move_char( ch, DIR_SOUTH, argument );
}

CMDRUNP( west )
{
    move_char( ch, DIR_WEST, argument );
}

CMDRUNP( up )
{
    move_char( ch, DIR_UP, argument );
}

CMDRUNP( down )
{
    move_char( ch, DIR_DOWN, argument );
}

/*--------------------------------------------------------------------
 *    scan 
 *-------------------------------------------------------------------*/
#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

#define CLR_SCAN_MOB(ch)  (MILD(ch) ? "w" : "W")
#define CLR_SCAN_DIR(ch)  (MILD(ch) ? "c" : "C")
#define CLR_SCAN_DOOR(ch) (MILD(ch) ? "w" : "W")

static void scan_people( Room *room, Character *ch, int depth, int door, 
                         bool fShowDir, ostringstream &buf )
{
    Character *rch, *orig;
    bool found;
    bool fRus = ch->getConfig( ).ruexits;

    found = false;
    
    for (rch = room->people; rch != 0; rch = rch->next_in_room) {
        if (rch == ch) 
            continue;
        if (rch->invis_level > ch->get_trust()) 
            continue;
        if (!ch->can_see( rch ))
            continue;

        if (!found) {
            buf << "{" << CLR_SCAN_DIR(ch);

            if (door != -1) {
                if (fShowDir)
                    buf << (fRus ? dirs[door].where : dirs[door].name);
                else
                    buf << "Дальность " << depth;
            }
            else
                buf << "Здесь";
            
            buf << ":{x" << endl;
            found = true;
        }

        orig = rch->getDoppel( ch );
        
        buf << "    {" << CLR_SCAN_MOB(ch) << ch->sees( orig, '1' ) << ".{x";

        if (IS_SET( orig->comm, COMM_AFK ))
            buf << " {w[{C{leAFK{lrАФК{x{w]{x";

        buf << endl; 
    }
}

static Room * scan_room( Room *start_room, Character *ch, int depth, int door,
                       bool fShowDir, ostringstream &buf )
{
    EXIT_DATA *pExit;
    Room *room;
    bool fRus = ch->getConfig( ).ruexits;

    pExit = start_room->exit[door];
    
    if (!pExit || !ch->can_see( pExit ))
        return NULL;
    
    room = pExit->u1.to_room;

    if (IS_SET(pExit->exit_info, EX_CLOSED)) {
        buf << "{" << CLR_SCAN_DIR(ch);

        if (fShowDir)
            buf << (fRus ? dirs[door].where : dirs[door].name);
        else
            buf << "Дальность " << depth;
        
        buf << ":{x" << endl
            << "    {" << CLR_SCAN_DOOR(ch) << russian_case(direction_doorname(pExit), '1') << " (закрыто).{x" << endl;

        return NULL;
    }

    if (IS_SET(pExit->exit_info, EX_NOSCAN)) {
        buf << "{" << CLR_SCAN_DIR(ch);

        if (fShowDir)
            buf << (fRus ? dirs[door].where : dirs[door].name);
        else
            buf << "Дальность " << depth;
        
        buf << ":{x" << endl << "    Невозможно что-либо разглядеть." << endl;
        return NULL;
    }
    
    scan_people( room, ch, depth, door, fShowDir, buf );
    return room;
}

CMDRUNP( scan )
{
    ostringstream buf;
    char arg1[MAX_INPUT_LENGTH];
    Room *room;
    int door, depth;
    int range;

    if (ch->position < POS_SLEEPING) {
        ch->pecho( "Ты ничего не видишь, кроме звезд..." );
        return;
    }

    if (ch->position == POS_SLEEPING) {
        ch->pecho( "Ты спишь и можешь видеть только сны." );
        return;
    }

    argument = one_argument(argument, arg1);

    if (arg1[0] == '\0')
    {
        oldact("$c1 осматривает все вокруг.", ch, 0, 0, TO_ROOM );
        buf << "Осмотревшись, ты видишь:" << endl;
        scan_people( ch->in_room, ch, 0, -1, true, buf );

        for (door = 0; door < DIR_SOMEWHERE; door++)
            scan_room( ch->in_room, ch, 1, door, true, buf );

        ch->send_to( buf );
        return;
    }

    door = direction_lookup( arg1 );

    if (door < 0) {
        ch->pecho( "В какую сторону?" );
        return;
    }

    oldact("Ты пристально смотришь $T.", ch, 0, dirs[door].leave, TO_CHAR );
    oldact("$c1 пристально смотрит $T.", ch, 0, dirs[door].leave, TO_ROOM );
    
    range = max( 1, ch->getModifyLevel() / 10 );
    room = ch->in_room;

    for (depth = 1; depth <= range; depth++) {
        room = scan_room( room, ch, depth, door, false, buf );

        if (!room)
            break;
    }
    
    if (!buf.str( ).empty( )) {
        ch->pecho( "Ты видишь:" );
        ch->send_to( buf );
    }
}

/*--------------------------------------------------------------------
 *  stand / wake / sit / rest / sleep 
 *-------------------------------------------------------------------*/
DLString oprog_msg( Object *obj, const char *tag )
{
    DLString msg;
    Scripting::IdRef ID_MSG( tag );
    Scripting::Register regObj, regObjIndex, reg;
    
    if (!FeniaManager::wrapperManager)
        return msg;

    regObj = FeniaManager::wrapperManager->getWrapper( obj );
    regObjIndex = FeniaManager::wrapperManager->getWrapper( obj->pIndexData );

    try { 
        reg = *regObj[ID_MSG];
        if (reg.type != Scripting::Register::NONE)
            msg = reg.toString( ); 
        
        if (msg.empty( )) {
            reg = *regObjIndex[ID_MSG];
            if (reg.type != Scripting::Register::NONE)
                msg = reg.toString( ); 
        }
    } 
    catch (const Exception &e) { 
        LogStream::sendWarning( ) << e.what( ) << endl;
    }
    
    return msg;
}

static bool oprog_msg_furniture( Object *obj, Character *ch, const char *tagRoom, const char *tagChar )
{
    DLString msgRoom, msgChar;
    
    msgRoom = oprog_msg( obj, tagRoom );
    msgChar = oprog_msg( obj, tagChar );
    
    if (msgChar.empty( ) && msgRoom.empty( ))
        return false;

    if (!msgChar.empty( ))
        ch->pecho( msgChar.c_str( ), obj, ch );

    if (!msgRoom.empty( ))
        ch->recho( POS_RESTING, msgRoom.c_str( ), obj, ch );

    return true;
}

CMDRUNP( stand )
{
        Object *obj = 0;
        
        if (argument[0] != '\0')
        {
                if (ch->position == POS_FIGHTING)
                {
                        ch->pecho( "Во время сражения есть дела поважнее." );
                        return;
                }

                obj = get_obj_list(ch,argument,ch->in_room->contents);

                if (obj == 0)
                {
                        ch->pecho( "Ты не видишь этого здесь." );
                        return;
                }

                if ( obj->item_type != ITEM_FURNITURE
                        || ( !IS_SET(obj->value2(),STAND_AT)
                                && !IS_SET(obj->value2(),STAND_ON)
                                && !IS_SET(obj->value2(),STAND_IN) ) )
                {
                        ch->pecho( "Ты не можешь стоять на этом." );
                        return;
                }

                if (ch->on != obj && count_users(obj) >= obj->value0())
                {
                        oldact_p("На $o6 нет свободного места.",
                                ch,obj,0,TO_ROOM,POS_DEAD);
                        return;
                }

                ch->on = obj;
        }

        switch ( ch->position.getValue( ) )
        {
        case POS_SLEEPING:
                if ( IS_AFFECTED(ch, AFF_SLEEP) )
                {
                        ch->pecho( "Ты не можешь проснуться!" );
                        return;
                }

                if (obj == 0)
                {
                        ch->pecho( "Ты просыпаешься и встаешь." );
                        oldact("$c1 просыпается и встает.", ch, 0, 0, TO_ROOM);
                        ch->on = 0;
                }
                else if (!oprog_msg_furniture( obj, ch, "msgWakeStandRoom", "msgWakeStandChar" )) {
                    if (IS_SET(obj->value2(),STAND_AT))
                    {
                            oldact_p("Ты просыпаешься и становишься возле $o2.",ch,obj,0,TO_CHAR,POS_DEAD);
                            oldact("$c1 просыпается и становится возле $o2.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),STAND_ON))
                    {
                            oldact_p("Ты просыпаешься и становишься на $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
                            oldact("$c1 просыпается и становится на $o4.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact_p("Ты просыпаешься и становишься в $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
                            oldact("$c1 просыпается и становится в $o4.",ch,obj,0,TO_ROOM);
                    }
                }

                if (IS_HARA_KIRI(ch))
                {
                        ch->pecho( "Ты чувствуешь, как рана от харакири затягивается, и твое тело заживает." );
                        REMOVE_BIT(ch->act,PLR_HARA_KIRI);
                }

                ch->position = POS_STANDING;
                interpret_raw(ch, "look", "auto");
                break;

        case POS_RESTING:
        case POS_SITTING:
        case POS_STANDING:
                if (obj == 0)
                {
                        if (ch->position == POS_STANDING) {
                                ch->pecho( "Ты уже стоишь." );
                        } else {
                                ch->pecho( "Ты встаешь." );
                                oldact("$c1 встает.", ch, 0, 0, TO_ROOM);
                        }
                        ch->on = 0;
                }
                else if (!oprog_msg_furniture( obj, ch, "msgStandRoom", "msgStandChar" )) {
                    if (IS_SET(obj->value2(),STAND_AT))
                    {
                            oldact("Ты становишься возле $o2.",ch,obj,0,TO_CHAR);
                            oldact("$c1 становится возле $o2.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),STAND_ON))
                    {
                            oldact("Ты становишься на $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 становится на $o4.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact("Ты становишься в $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 становится в $o4.",ch,obj,0,TO_ROOM);
                    }
                }

                ch->position = POS_STANDING;
                break;

        case POS_FIGHTING:
                ch->pecho( "Ты уже сражаешься!" );
                ch->on = 0;
                break;
        }
}



CMDRUNP( rest )
{
        Object *obj = 0;

        if (ch->position == POS_FIGHTING)
        {
                ch->pecho( "Во время сражения есть дела поважнее." );
                return;
        }

        if (MOUNTED(ch))
        {
                ch->pecho( "Ты не можешь отдыхать, когда ты в седле." );
                return;
        }

        if (RIDDEN(ch))
        {
                ch->pecho("Ты не можешь отдыхать, когда ты оседлан%Gо||а.", ch);
                return;
        }

        if ( IS_AFFECTED(ch, AFF_SLEEP) )
        {
                ch->pecho( "Ты спишь и не можешь проснуться." );
                return;
        }

        if ( ch->death_ground_delay > 0
                && ch->trap.isSet( TF_NO_MOVE ) )
        {
                ch->pecho( "Тебе некогда отдыхать." );
                return;
        }

        /* okay, now that we know we can rest, find an object to rest on */
        if (argument[0] != '\0')
        {
                obj = get_obj_list(ch,argument,ch->in_room->contents);

                if (obj == 0)
                {
                        ch->pecho( "Ты не видишь этого здесь." );
                        return;
                }
        }
        else
                obj = ch->on;

        if (obj != 0)
        {
                if ( ( obj->item_type != ITEM_FURNITURE )
                        || ( !IS_SET(obj->value2(),REST_ON)
                                && !IS_SET(obj->value2(),REST_IN)
                                && !IS_SET(obj->value2(),REST_AT) ) )
                {
                        ch->pecho( "Ты не можешь отдыхать на этом." );
                        return;
                }

                if (obj != 0 && ch->on != obj && count_users(obj) >= obj->value0())
                {
                        oldact_p("На $o6 нет свободного места.",ch,obj,0,TO_CHAR,POS_DEAD);
                        return;
                }

                ch->on = obj;
        }
        
        switch ( ch->position.getValue( ) )
        {
        case POS_SLEEPING:
                if (DIGGED(ch)) {
                    ch->pecho( "Ты просыпаешься." );
                } 
                else if (obj == 0)
                {
                        ch->pecho( "Ты просыпаешься и садишься отдыхать." );
                        oldact("$c1 просыпается и садится отдыхать.",ch,0,0,TO_ROOM);
                }
                else if (!oprog_msg_furniture( obj, ch, "msgWakeRestRoom", "msgWakeRestChar" )) {
                    if (IS_SET(obj->value2(),REST_AT))
                    {
                            oldact_p("Ты просыпаешься и садишься отдыхать возле $o2.",
                                    ch,obj,0,TO_CHAR,POS_SLEEPING);
                            oldact("$c1 просыпается и садится отдыхать возле $o2.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),REST_ON))
                    {
                            oldact_p("Ты просыпаешься и садишься отдыхать на $o4.",
                                    ch,obj,0,TO_CHAR,POS_SLEEPING);
                            oldact("$c1 просыпается и садится отдыхать на $o4.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact_p("Ты просыпаешься и садишься отдыхать в $o4.",
                                    ch,obj,0,TO_CHAR,POS_SLEEPING);
                            oldact("$c1 просыпается и садится отдыхать в $o4.",ch,obj,0,TO_ROOM);
                    }
                }
                ch->position = POS_RESTING;
                break;

        case POS_RESTING:
                ch->pecho( "Ты уже отдыхаешь." );
                break;

        case POS_STANDING:
                if (obj == 0)
                {
                        ch->pecho( "Ты садишься отдыхать." );
                        oldact("$c1 садится отдыхать.", ch, 0, 0, TO_ROOM);
                }
                else if (!oprog_msg_furniture( obj, ch, "msgSitRestRoom", "msgSitRestChar" )) {
                    if (IS_SET(obj->value2(),REST_AT))
                    {
                            oldact("Ты садишься возле $o2 и отдыхаешь.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится возле $o2 и отдыхает.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),REST_ON))
                    {
                            oldact("Ты садишься на $o4 и отдыхаешь.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится на $o4 и отдыхает.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact("Ты садишься отдыхать в $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится отдыхать в $o4.",ch,obj,0,TO_ROOM);
                    }
                }
                ch->position = POS_RESTING;
                break;

        case POS_SITTING:
                if (obj == 0)
                {
                        ch->pecho( "Ты отдыхаешь." );
                        oldact("$c1 отдыхает.",ch,0,0,TO_ROOM);
                }
                else if (!oprog_msg_furniture( obj, ch, "msgRestRoom", "msgRestChar" )) {
                    if (IS_SET(obj->value2(),REST_AT))
                    {
                            oldact("Ты отдыхаешь возле $o2.",ch,obj,0,TO_CHAR);
                            oldact("$c1 отдыхает возле $o2.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),REST_ON))
                    {
                            oldact("Ты отдыхаешь на $o6.",ch,obj,0,TO_CHAR);
                            oldact("$c1 отдыхает на $o6.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact("Ты отдыхаешь в $o6.",ch,obj,0,TO_CHAR);
                            oldact("$c1 отдыхает в $o6.",ch,obj,0,TO_ROOM);
                    }
                }
                ch->position = POS_RESTING;

                if (IS_HARA_KIRI(ch))
                {
                        ch->pecho( "Ты чувствуешь, как рана от харакири затягивается, и твое тело заживает." );
                        REMOVE_BIT(ch->act,PLR_HARA_KIRI);
                }

                break;
        }

        return;
}


CMDRUNP( sit )
{
        Object *obj = 0;

        if (ch->position == POS_FIGHTING)
        {
                ch->pecho( "Во время сражения есть дела поважнее." );
                return;
        }

        if (MOUNTED(ch))
        {
                ch->pecho( "Ты не можешь сесть, когда ты в седле." );
                return;
        }

        if (RIDDEN(ch))
        {
                ch->pecho("Ты не можешь сесть, когда ты оседлан%Gо||а.", ch);
                return;
        }

        if ( IS_AFFECTED(ch, AFF_SLEEP) )
        {
                ch->pecho( "Ты спишь и не можешь проснуться." );
                return;
        }

        if ( ch->death_ground_delay > 0
                && ch->trap.isSet( TF_NO_MOVE ) )
        {
                ch->pecho( "Тебе не до отдыха!" );
                return;
        }

    /* okay, now that we know we can sit, find an object to sit on */
        if (argument[0] != '\0')
        {
                obj = get_obj_list(ch,argument,ch->in_room->contents);

                if (obj == 0)
                {
                        if ( IS_AFFECTED(ch, AFF_SLEEP) )
                        {
                                ch->pecho( "Ты спишь и не можешь проснуться." );
                                return;
                        }

                        ch->pecho( "Ты не видишь этого здесь." );
                        return;
                }
        }
        else
                obj = ch->on;

        if (obj != 0)
        {
                if ( ( obj->item_type != ITEM_FURNITURE )
                        || ( !IS_SET(obj->value2(),SIT_ON)
                                && !IS_SET(obj->value2(),SIT_IN)
                                && !IS_SET(obj->value2(),SIT_AT) ) )
                {
                        ch->pecho( "Ты не можешь сесть на это." );
                        return;
                }

                if (obj != 0 && ch->on != obj && count_users(obj) >= obj->value0())
                {
                        oldact_p("На $o6 нет больше свободного места.",ch,obj,0,TO_CHAR,POS_DEAD);
                        return;
                }

                ch->on = obj;
        }

        switch (ch->position.getValue( ))
        {
        case POS_SLEEPING:
                if (obj == 0)
                {
                        ch->pecho( "Ты просыпаешься и садишься." );
                        oldact("$c1 просыпается и садится.", ch, 0, 0, TO_ROOM);
                }
                else if (!oprog_msg_furniture( obj, ch, "msgWakeSitRoom", "msgWakeSitChar" )) {
                    if (IS_SET(obj->value2(),SIT_AT))
                    {
                            oldact_p("Ты просыпаешься и садишься возле $o2.",ch,obj,0,TO_CHAR,POS_DEAD);
                            oldact("$c1 просыпается и садится возле $o2.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),SIT_ON))
                    {
                            oldact_p("Ты просыпаешься и садишься на $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
                            oldact("$c1 просыпается и садится на $o4.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact_p("Ты просыпаешься и садишься в $o4.",ch,obj,0,TO_CHAR,POS_DEAD);
                            oldact("$c1 просыпается и садится в $o4.",ch,obj,0,TO_ROOM);
                    }
                }

                ch->position = POS_SITTING;
                break;

        case POS_RESTING:
                if (obj == 0)
                        ch->pecho( "Ты прекращаешь отдых." );
                else if (!oprog_msg_furniture( obj, ch, "msgSitRoom", "msgSitChar" )) {
                    if (IS_SET(obj->value2(),SIT_AT))
                    {
                            oldact("Ты садишься возле $o2.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится возле $o2.",ch,obj,0,TO_ROOM);
                    }

                    else if (IS_SET(obj->value2(),SIT_ON))
                    {
                            oldact("Ты садишься на $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится на $o4.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact("Ты садишься в $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится в $o4.",ch,obj,0,TO_ROOM);
                    }
                }

                ch->position = POS_SITTING;
                break;

        case POS_SITTING:
                ch->pecho( "Ты уже сидишь." );
                break;

        case POS_STANDING:
                if (obj == 0)
                {
                        ch->pecho( "Ты садишься." );
                        oldact("$c1 садится на землю.",ch,0,0,TO_ROOM);
                }
                else if (!oprog_msg_furniture( obj, ch, "msgSitRoom", "msgSitChar" )) {
                    if (IS_SET(obj->value2(),SIT_AT))
                    {
                            oldact("Ты садишься возле $o2.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится возле $o2.",ch,obj,0,TO_ROOM);
                    }
                    else if (IS_SET(obj->value2(),SIT_ON))
                    {
                            oldact("Ты садишься на $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится на $o4.",ch,obj,0,TO_ROOM);
                    }
                    else
                    {
                            oldact("Ты садишься в $o4.",ch,obj,0,TO_CHAR);
                            oldact("$c1 садится в $o4.",ch,obj,0,TO_ROOM);
                    }
                }
                ch->position = POS_SITTING;
                break;
        }

        if (IS_HARA_KIRI(ch))
        {
                ch->pecho( "Ты чувствуешь, как рана от харакири затягивается, и твое тело заживает." );
                REMOVE_BIT(ch->act,PLR_HARA_KIRI);
        }
        return;
}

GSN(curl);

CMDRUNP( sleep )
{
    Object *obj = 0;
    ostringstream toMe, toRoom;

    if (MOUNTED(ch))
    {
            ch->pecho( "Ты не можешь спать, когда ты в седле." );
            return;
    }

    if (RIDDEN(ch))
    {
            ch->pecho("Ты не можешь спать, когда ты оседлан%Gо||а.", ch);
            return;
    }

    if ( ch->death_ground_delay > 0
            && ch->trap.isSet( TF_NO_MOVE ) )
    {
            ch->pecho( "Тебе не до сна!" );
            return;
    }

    switch ( ch->position.getValue( ) ) {
    case POS_SLEEPING:
        ch->pecho( "Ты уже спишь." );
        return;

    case POS_FIGHTING:
        ch->pecho( "Но ты же сражаешься!" );
        return;

    case POS_RESTING:
    case POS_SITTING:
    case POS_STANDING:
        if (argument[0] == '\0' && ch->on == 0)
        {
            ch->position = POS_SLEEPING;

            toMe << "Ты засыпаешь";
            toRoom << "%1$^C1 засыпает";
            
            if (gsn_curl->getEffective( ch ) > 1 ) {
                toMe << ", свернувшись клубочком";
                toRoom << ", свернувшись клубочком";
            }
        }
        else  /* find an object and sleep on it */
        {
            if (argument[0] == '\0')
                    obj = ch->on;
            else
                    obj = get_obj_list( ch, argument,  ch->in_room->contents );

            if (obj == 0)
            {
                    ch->pecho( "Ты не видишь этого здесь." );
                    return;
            }

            if ( obj->item_type != ITEM_FURNITURE
                    || ( !IS_SET(obj->value2(),SLEEP_ON)
                            && !IS_SET(obj->value2(),SLEEP_IN)
                            && !IS_SET(obj->value2(),SLEEP_AT)))
            {
                    ch->pecho( "Ты не можешь спать на этом!" );
                    return;
            }

            if (ch->on != obj && count_users(obj) >= obj->value0())
            {
                    oldact_p("На $o6 не осталось свободного места для тебя.",
                            ch,obj,0,TO_CHAR,POS_DEAD);
                    return;
            }

            ch->on = obj;
            ch->position = POS_SLEEPING;

            if (oprog_msg_furniture( obj, ch, "msgSleepRoom", "msgSleepChar" ))
                return;

            toMe << "Ты ложишься спать ";
            toRoom << "%1$^C1 ложится спать ";

            if (IS_SET(obj->value2(),SLEEP_AT))
            {
                toMe << "возле %2$O2";
                toRoom << "возле %2$O2";
            }
            else if (IS_SET(obj->value2(),SLEEP_ON))
            {
                toMe << "на %2$O4";
                toRoom << "на %2$O4";
            }
            else
            {
                toMe << "в %2$O4";
                toRoom << "в %2$O4";
            }
            
            if (gsn_curl->getEffective( ch ) > 1 ) {
                toMe << ", свернувшись клубочком";
                toRoom << ", свернувшись клубочком";
            }

        }
        break;
    }

    toMe << ".";
    toRoom << ".";
    ch->pecho( toMe.str( ).c_str( ), ch, obj );
    ch->recho( POS_RESTING, toRoom.str( ).c_str( ), ch, obj );
}

static bool mprog_wake( Character *ch, Character *waker )
{
    FENIA_CALL( ch, "Wake", "C", waker );
    FENIA_NDX_CALL( ch->getNPC( ), "Wake", "CC", ch, waker );
    return false;
}

/* COMPAT */ void do_stand( Character *ch, const char *argument )
{
    interpret_raw( ch, "stand", argument ); 
}

CMDRUNP( wake )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );
    
    if ( arg[0] == '\0' ) { 
        if (DIGGED(ch) && ch->position <= POS_SLEEPING)
            interpret_raw( ch, "rest", argument );
        else {
            undig( ch );
            do_stand( ch, argument );
        }

        return; 
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 ) { 
        ch->pecho( "Этого нет здесь." ); 
        return; 
    }

    if (ch == victim) { 
        ch->pecho( "Ты не можешь разбудить сам{Sfа{Sx себя!" ); 
        return; 
    }

    if (IS_AWAKE(victim)) { 
        oldact("$C1 уже не спит.", ch, 0, victim, TO_CHAR); 
        return; 
    }

    if (IS_AFFECTED(victim, AFF_SLEEP)) { 
        oldact("Ты не можешь разбудить $S от нездорового сна!", ch, 0, victim, TO_CHAR);  
        return; 
    }

    oldact("Ты будишь $C4.", ch, 0, victim, TO_CHAR);
    oldact("$c1 будит $C4.", ch, 0, victim, TO_NOTVICT);
    oldact_p("$c1 будит тебя.", ch, 0, victim, TO_VICT,POS_SLEEPING );
    do_stand(victim,"");
    mprog_wake( victim, ch );
}


/*
 * Contributed by Alander
 */
CMDRUNP( visible )
{
    do_visible( ch );
}


CMDRUNP( flyup )
{
    interpret_raw( ch, "fly", "up" );
}

CMDRUNP( flydown )
{
    interpret_raw( ch, "fly", "down" );
}

CMDRUNP( fly )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (!str_cmp(arg,"up") || !str_cmp(arg,"вверх"))
    {
        if (!can_fly( ch )) {
            ch->pecho( "Чтобы взлететь, найди крылья или зелье." );
            return;
        }

        if (!ch->posFlags.isSet( POS_FLY_DOWN )) {
            ch->pecho( "Ты уже летаешь." );
            return;
        }

        ch->posFlags.removeBit( POS_FLY_DOWN );
        ch->pecho( "Ты начинаешь летать." );
        ch->recho( "%^C1 начинает летать.", ch );
    }
    else if (!str_cmp(arg,"down") || !str_cmp(arg,"вниз"))
    {
        if (!is_flying( ch )) {
            ch->pecho( "Твои ноги уже на земле." );
            return;
        }

        ch->posFlags.setBit( POS_FLY_DOWN );
        ch->pecho( "Твои ноги медленно опускаются на землю." );
        ch->recho( "%^C1 медленно опускается на землю.", ch );
    }
    else
    {
        ch->pecho( "Напиши {y{hc{lefly up{lrвзлететь{x или {y{hc{lefly down{lrнелетать{x." );
        return;
    }

    ch->setWait( gsn_fly->getBeats(ch) );
}


/*
 * Экстра-выходы - разработка Тирна.
 */
CMDRUNP( enter )
{
    Object *portal = 0;
    EXTRA_EXIT_DATA *peexit = 0;
    
    // Syntax: enter, enter <portal>, enter <eexit>.
    if (!argument[0])
        portal = get_obj_room_type( ch, ITEM_PORTAL );
    else {
        portal = get_obj_list( ch, argument, ch->in_room->contents );
        peexit = ch->in_room->extra_exits.find(argument);
    }

    if (portal == 0 && peexit == 0) {
        if (!argument[0])
            ch->pecho( "Куда ты хочешь войти?");             
        else
            ch->pecho( "Ты не видишь здесь такого портала или дополнительного выхода.");           
        return;
    }
    
    if (peexit) {
        move_char( ch, peexit );
        return;            
    }

    if (portal->item_type != ITEM_PORTAL) {
        ch->pecho( "Ты не находишь пути внутрь %O2, это не портал.", portal );
        return;
    }

    PortalMovement( ch, portal ).move( );
}


