#include "commandtemplate.h"
#include "character.h"
#include "loadsave.h"
#include "core/object.h"
#include "liquid.h"
#include "wrappertarget.h"
#include "wrapperbase.h"
#include "core/behavior/behavior_utils.h"
#include "keyhole.h"
#include "terrains.h"
#include "wearloc_utils.h"
#include "msgformatter.h"
#include "act.h"
#include "merc.h"
#include "def.h"

DLString get_pocket_argument( char *arg );
void show_char_to_char_1( Character *victim, Character *ch, bool fBrief );
void show_list_to_char( Object *list, Character *ch, bool fShort, 
                        bool fShowNothing, DLString pocket = "", Object *container = NULL );


/*---------------------------------------------------------------------------
 * examine object trigger
 *--------------------------------------------------------------------------*/
static bool oprog_examine_money( Object *obj, Character *ch, const DLString& )
{
    if (obj->value0() == 0)
    {
        if (obj->value1() == 0)
                ch->pecho("Жаль, но здесь нет золота.");
        else if (obj->value1() == 1)
                ch->pecho("Ух ты! Одна золотая монетка!");
        else
                ch->pecho("Здесь %d золот%s.",
                        obj->value1(),GET_COUNT(obj->value1(), "ая монета","ые монеты","ых монет"));
    }
    else if (obj->value1() == 0)
    {
        if (obj->value0() == 1)
                ch->pecho("Ух ты! Одна серебряная монетка.");
        else
                ch->pecho("Здесь %d серебрян%s.",
                        obj->value0(),GET_COUNT(obj->value0(), "ая монета","ые монеты","ых монет"));
    }
    else
        ch->pecho("Здесь %d золот%s и %d серебрян%s.",
                obj->value1(),GET_COUNT(obj->value1(), "ая","ые","ых"),
                obj->value0(),GET_COUNT(obj->value0(), "ая монета","ые монеты","ых монет"));
    return true;
}

static bool oprog_examine_drink_container( Object *obj, Character *ch, const DLString& )
{
    if (IS_SET(obj->value3(), DRINK_CLOSED)) {
        ch->pecho("Эта емкость закрыта.");
        return true;
    }

    if (obj->value1() <= 0) {
        ch->pecho( "Тут пусто." );
        return true;
    }

    ch->pecho( "%s наполнен жидкостью %s цвета.",
                obj->value1() < obj->value0() / 4 ? 
                    "Меньше, чем до половины" :
                    obj->value1() < 3 * obj->value0() / 4 ? 
                        "До половины"  : 
                        "Чуть более, чем наполовину",
                liquidManager->find( obj->value2() )->getColor( ).ruscase( '2' ).c_str( )
              );
    return true;
}

static bool oprog_examine_portal( Object *obj, Character *ch, const DLString &pocket )
{
    return PortalKeyhole( ch, obj ).doExamine( );
}

static bool oprog_examine_container( Object *obj, Character *ch, const DLString &pocket )
{
    ContainerKeyhole( ch, obj ).doExamine( );

    if (IS_SET(obj->value1(), CONT_LOCKED)) {
        if (obj->behavior && !obj->behavior->canLock(ch) && obj->value2() <= 0)
            ch->pecho("%^O1 -- чья-то личная собственность, отпереть сможет только хозяин или хозяйка.", obj);
        else
            ch->pecho("%1$^O1 заперт%1$Gо||а|ы на ключ, сперва отопри.", obj);
        return true;
    }

    if (IS_SET(obj->value1(), CONT_CLOSED)) {
        ch->pecho("%1$^O1 закрыт%1$Gо||а|ы, сперва открой.", obj);
        return true;
    }
    
    if (!pocket.empty( )) {
        if (!IS_SET(obj->value1(), CONT_WITH_POCKETS)) {
            ch->pecho( "Ты не видишь здесь ни одного кармана." );
            return true;
        }
    }
    
    const char *p = pocket.c_str( );
    const char *where;
    if (obj->in_room)
        where = terrains[obj->in_room->getSectorType()].where;
    else if (obj->wear_loc == wear_none)
        where = "в твоих руках";
    else
        where = "в твоей экипировке";

    if (IS_SET(obj->value1(),CONT_PUT_ON|CONT_PUT_ON2)) {
        if (!pocket.empty( ))
            ch->pecho( "Отделение '%2$s' %1$O2 содержит:", obj, p );
        else
            ch->pecho( "На %1$O6 ты видишь:", obj );
    }
    else {
        if (!pocket.empty( )) {
            if (!obj->can_wear( ITEM_TAKE ))
                ch->pecho( "На полке %1$O2 с надписью '%2$s' ты видишь:", obj, p );
            else
                ch->pecho( "В кармане %1$O2 с надписью '%2$s' ты видишь:", obj, p );
        }
        else {
            ch->pecho( "%1$^O1 %2s содерж%1$nит|ат:", obj, where );
        }
    }

    show_list_to_char( obj->contains, ch, true, true, pocket, obj );
    return true;
}    

static bool oprog_examine_corpse( Object *obj, Character *ch, const DLString & )
{
    oldact("На $o6 ты видишь:", ch, obj, 0, TO_CHAR );
    show_list_to_char( obj->contains, ch, true, true );
    return true;
}        

static bool oprog_examine_keyring( Object *obj, Character *ch, const DLString & )
{
    oldact("На $o4 нанизано:", ch, obj, 0, TO_CHAR );
    show_list_to_char( obj->contains, ch, true, true );
    return true;
}        

static bool oprog_examine( Object *obj, Character *ch, const DLString &arg )
{
    FENIA_CALL( obj, "Examine", "Cs", ch, arg.c_str( ) );
    FENIA_NDX_CALL( obj, "Examine", "OCs", obj, ch, arg.c_str( ) );
    BEHAVIOR_CALL( obj, examine, ch );
   
    bool rc = false;
    switch (obj->item_type) {
    case ITEM_MONEY:
        rc = oprog_examine_money( obj, ch, arg );
        break;

    case ITEM_DRINK_CON:
        rc = oprog_examine_drink_container( obj, ch, arg );
        break;
        
    case ITEM_CONTAINER:
        rc = oprog_examine_container( obj, ch, arg );
        break;

    case ITEM_KEYRING:
        rc = oprog_examine_keyring( obj, ch, arg );
        break;
        
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
        rc = oprog_examine_corpse( obj, ch, arg );
        break;

    case ITEM_PORTAL:
        rc = oprog_examine_portal( obj, ch, arg );
        break;
    }

    if (!rc)
        ch->pecho( "Внутрь %O2 невозможно заглянуть.", obj );

    return rc;
}

/*-------------------------------------------------------------------------
 * 'examine' helper function, also called from 'look' 
 *-------------------------------------------------------------------------*/
void do_look_into( Character *ch, char *arg2 )
{
    DLString pocket;
    Object *obj;
    
    if (arg2[0] == '\0')
    {
        ch->pecho( "Посмотреть на что?" );
        return;
    }
    
    pocket = get_pocket_argument( arg2 );
    obj = get_obj_here( ch, arg2 );

    if (!obj) {
        ch->pecho( "Ты не видишь этого тут." );
        return;
    }
    
    oprog_examine( obj, ch, pocket );
}

/*-------------------------------------------------------------------------
 * 'examine' command 
 *-------------------------------------------------------------------------*/
CMDRUNP( examine )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if (eyes_blinded( ch )) {
        eyes_blinded_msg( ch );
        return;
    }

    argument = one_argument( argument, arg );

    if (arg[0] == '\0') {
        ch->pecho( "Изучить что?" );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) != 0 )
    {
        if ( victim->can_see( ch ) )
        {
            if (ch == victim)
                oldact("$c1 осматривает себя.",ch,0,0,TO_ROOM);
            else
            {
                oldact("$c1 бросает взгляд на тебя.", ch, 0, victim, TO_VICT);
                oldact("$c1 бросает взгляд на $C4.",  ch, 0, victim, TO_NOTVICT);
            }
        }

        show_char_to_char_1( victim, ch, true );
        return;
    }

    do_look_into(ch, arg);
}


