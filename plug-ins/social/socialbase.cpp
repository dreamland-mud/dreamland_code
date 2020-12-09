/* $Id: socialbase.cpp,v 1.1.2.3.6.8 2009/08/16 02:50:31 rufina Exp $
 * 
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */

#include "socialbase.h"

#include "russianstring.h"
#include "skillreference.h"
#include "character.h"
#include "room.h"

#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"

GSN(improved_invis);

SocialBase::SocialBase( ) 
{
}

SocialBase::~SocialBase( )
{
}

short SocialBase::getLog( ) const
{
    return LOG_NORMAL;
}

bool SocialBase::matches( const DLString& argument ) const
{
    if (argument.empty( )) 
        return false;

    if (argument.strPrefix( getName( ) )) 
        return true;
    
    if (argument.strPrefix( getRussianName( ) )) 
        return true;

    return false;
}

bool SocialBase::properOrder( Character * )
{
    return true;
}

bool SocialBase::dispatchOrder( const InterpretArguments &iargs )
{
    return dispatch( iargs );
}

bool SocialBase::dispatch( const InterpretArguments &iargs )
{
    Character *ch = iargs.ch;

    if (!ch->is_npc( )) {
        if (IS_SET(ch->act, PLR_FREEZE)) {
            ch->pecho("Ты полностью замороже%Gно|н|на!", ch);
            return false;
        }

        if (IS_SET( ch->comm, COMM_NOEMOTE )) {
            ch->pecho("Ты анти-социал%Gьно|ен|ьна!", ch);
            return false;
        }

        if (IS_SET( ch->comm, COMM_AFK )) {
            ch->send_to( "Выйди сначала из {WAFK{x\n\r" );
            return false;
        }
    }
    
    if (!checkPosition( ch )) 
        return false;
    
    visualize( ch );
    return true;
}

static const void *victimOrSelf(Character *ch, Character *victim)
{
    static RussianString self("с||ебя||ебя||ебе||ебя||обой||ебе");
    if (ch == victim)
        return &self;
    else
        return victim;
}

enum {
    RC_NOARG,
    RC_VICT,
    RC_SELF,
    RC_OBJ,
    RC_VICT_NOT_FOUND,
    RC_VICT_VICT,
    RC_VICT_OBJ,
    RC_VICT2_NOT_FOUND
};

static int parseArguments(Character *ch, const DLString &arg1, const DLString &arg2, 
                                Character *&victim, Character *&victim2, Object *&obj)
{
    victim = 0;
    victim2 = 0;
    obj = 0;

    // No arguments specified: RC_NOARG.
    if (arg1.empty())
        return RC_NOARG;

    // Only one argument. Allowed syntax:
    // <social> <char1|self>      : RC_VICT, RC_SELF
    // <social> <char not found>  : RC_VICT_NOT_FOUND
    // <social> <obj>             : RC_OBJ
    if (arg2.empty()) {
        victim = get_char_room(ch, arg1);
        if (!victim)
            obj = get_obj_here(ch, arg1);

        if (victim == ch)
            return RC_SELF;
        else if (victim)
            return RC_VICT;
        else if (obj)
            return RC_OBJ;
        else
            return RC_VICT_NOT_FOUND;
    }

    // Two arguments. Allowed syntax:
    // <social> <char1|self> <char2|self>    : RC_VICT_VICT, RC_SELF, RC_VICT
    // <social> <char|self> <obj>            : RC_VICT_OBJ
    // <social> <char|not found> <not found> : RC_VICT_NOT_FOUND, RC_VICT2_NOT_FOUND
    victim = get_char_room(ch, arg1);
    victim2 = get_char_room(ch, arg2);
    if (!victim2)
        obj = get_obj_here(ch, arg2);

    if (!victim)
        return RC_VICT_NOT_FOUND;
    else if (obj)
        return RC_VICT_OBJ;
    else if (victim2 == ch && victim == ch)
        return RC_SELF;
    else if (victim2 == victim)
        return RC_VICT;
    else if (victim2)
        return RC_VICT_VICT;
    else
        return RC_VICT2_NOT_FOUND;
}

/**
 * Parse arguments and execute the social. Accepted combination of arguments:
 *
 * <none>                    : msgCharNoArgument, msgOthersNoArgument
 * <char1>                   : msgCharFound, msgVictimFound, msgOthersFound
 * <char not found>          : msgCharNotFound
 * <self>                    : msgCharAuto, msgOthersAuto
 * <char1|self> <char2|self> : msgCharFound2, msgVictimFound2, msgOthersFound2
 *                             hard-coded msg for not found
 * <obj>                     : msgCharObj, msgOthersObj
 * <char|self> <obj>         : msgCharVictimObj, msgVictimObj, msgOthersVictimObj
 */
void SocialBase::run( Character *ch, const DLString &constArguments )
{
    Character *victim, *victim2;
    const void *arg1, *arg2;
    Object *obj;
    DLString argument = constArguments;
    DLString firstArgument =  argument.getOneArgument( );
    DLString secondArgument = argument.getOneArgument( );
    int rc = parseArguments(ch, firstArgument, secondArgument, victim, victim2, obj);
    int pos = getPosition( );

    switch (rc) {
        case RC_NOARG:  // вызов без параметров
            act( getNoargOther( ).c_str( ), ch, 0, victim, TO_ROOM );
            act_p( getNoargMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
            break;
    
        case RC_SELF: // применение социала на себя, в т.ч. если оба аргумента - тоже я
            act( getAutoOther( ).c_str( ), ch, 0, victim, TO_ROOM );
            act_p( getAutoMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
            break;

        case RC_VICT: // применение социала на жертву, в т.ч. если оба аргумента - одна и та же жертва
            act( getArgOther( ).c_str( ), ch, 0, victim, TO_NOTVICT );
            act_p( getArgMe( ).c_str( ), ch, 0, victim, TO_CHAR, pos );
            act( getArgVictim( ).c_str( ), ch, 0, victim, TO_VICT );
            break;

        case RC_VICT_NOT_FOUND: // не найдет персонаж или предмет по первому аргументу
            // Handled after mobprogs had a chance to run.
            break;

        case RC_VICT2_NOT_FOUND: // не найден персонаж или предмет по второму аргументу
            if (victim == ch)
                ch->pecho( "Ты видишь только себя здесь, кто такой %s?", secondArgument.c_str( ));
            else
                ch->pecho( "Ты видишь только %1$C4 здесь, кто такой %s?", victim, secondArgument.c_str( ));
            return;


        case RC_VICT_VICT:
            // Output to actor and both victims. Substitute actor name with "self" if it matches victim.
            arg1 = victimOrSelf(ch, victim);
            arg2 = victimOrSelf(ch, victim2);

            ch->pecho( getArgMe2( ).c_str( ), ch, arg1, arg2 );
            if (victim != ch ) victim->pecho( getArgVictim2( ).c_str( ), ch, arg1, arg2 );
            if (victim2 != ch ) victim2->pecho( getArgVictim2( ).c_str( ), ch, arg2, arg1 );

            // Output to everyone else in the room.
            for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
                if (rch != ch && rch != victim && rch != victim2)
                    rch->pecho( getArgOther2( ).c_str( ), ch, arg1, arg2 );
            break;

        case RC_VICT_OBJ:
            arg1 = victimOrSelf(ch, victim);
            ch->pecho(getObjChar().c_str(), ch, arg1, obj);
            if (victim != ch)
                victim->pecho(getObjVictim().c_str(), ch, arg1, obj);
            ch->recho(victim, getObjOthers().c_str(), ch, arg1, obj);
            break;

        case RC_OBJ:
            ch->pecho(getObjNoVictimSelf().c_str(), ch, obj);
            ch->recho(getObjNoVictimOthers().c_str(), ch, obj);
            break;
    }
    
    bool reacted = reaction( ch, victim, firstArgument );
    if (!reacted && rc == RC_VICT_NOT_FOUND) {
        if (!getErrorMsg( ).empty( ))
            act_p( getErrorMsg( ).c_str( ), ch, 0, 0, TO_CHAR, getPosition( ) );
        else
            ch->println("Нет этого здесь.");
        return;
    }

    if (victim2 && victim2 != victim)
        reaction( ch, victim2, secondArgument );
}

bool SocialBase::checkPosition( Character *ch )
{
    if (ch->position >= getPosition( ))
        return true;

    switch (ch->position.getValue( )) {
    case POS_DEAD:
        ch->send_to("Лежи смирно! Ты {RТРУП{x.\n\r");
        break;

    case POS_INCAP:
    case POS_MORTAL:
        ch->send_to("Даже не думай об этом! Ты в ужасном состоянии.\n\r");
        break;

    case POS_STUNNED:
        ch->send_to("Ты не в состоянии сделать это.\n\r");
        break;

    case POS_SLEEPING:
        ch->send_to("Во сне? Или может сначала проснешься...\n\r");
        break;

    case POS_RESTING:
        ch->send_to( "Уфф... Но ведь ты отдыхаешь...\n\r" );
        break;

    case POS_SITTING:
        ch->send_to( "Сидя? Или может сначала встанешь...\n\r" );
        break;

    case POS_FIGHTING:
        act_p( "Тебе не до того, ты же сражаешься!", ch, 0, 0, TO_CHAR, POS_FIGHTING );
        break;
    }

    return false;
}

void SocialBase::visualize( Character *ch )                                        
{
    if (IS_AFFECTED( ch, AFF_HIDE|AFF_FADE ))  {
        REMOVE_BIT( ch->affected_by, AFF_HIDE|AFF_FADE );
        ch->send_to("Ты выходишь из тени.\n\r");
        act_p( "$c1 выходит из тени.", ch, 0, 0, TO_ROOM,POS_RESTING);
    }

    if (IS_AFFECTED(ch, AFF_IMP_INVIS)) {
        affect_strip(ch,gsn_improved_invis);
        act("Ты становишься видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_CHAR);
        act("$c1 становится видим$gо|ым|ой для окружающих.\n\r", ch,0,0,TO_ROOM);
    }
}
