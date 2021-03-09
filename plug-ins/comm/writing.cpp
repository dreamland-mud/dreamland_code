/* $Id: writing.cpp,v 1.1.2.9.6.3 2007/09/11 00:27:23 rufina Exp $
 *
 * ruffina, 2004
 */

#include "writing.h"

#include "class.h"
#include "logstream.h"

#include "pcharacter.h"
#include "object.h"

#include "act.h"
#include "fight.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"
#include "def.h"

#define log(x) LogStream::sendNotice() << x << endl

COMMAND(CWrite, "write")
{
    Object *target;
    DLString arguments = constArguments;
    DLString targetName = arguments.getOneArgument( );
    
    if (targetName.empty( )) {
        usage( ch );
        return;
    }
    
    if (( target = get_obj_carry( ch, targetName.c_str( ) ) ))
        writeOnPaper( ch, target, arguments );
    else if (( target = get_obj_room( ch, targetName.c_str( ) ) ))
        writeOnWall( ch, target, arguments );
    else
        ch->pecho("У тебя нет этого.");
}

void CWrite::writeOnWall( Character *ch, Object *wall, DLString &arguments )
{
    EXTRA_DESCR_DATA *ed;
    DLString cmd;
    Object *nail;

    if (wall->item_type != ITEM_PARCHMENT) {
        act("Тебе не удастся ничего нацарапать на %3$O6.", ch, wall, 0, TO_CHAR);
        return;
    }

    if (wall->can_wear(ITEM_TAKE)) {
        act("Подними %3$O4 с земли, а потом пиши.", ch, wall, 0, TO_CHAR);
        return;
    }
    
    if (!( nail = findNail( ch ) )) {
        ch->pecho("Ты не держишь в руках ничего колющего или царапающего.");
        return;
    }

    if (arguments.empty( )) {
        act("Что именно ты хочешь выцарапать на %3$O6?", ch, wall, 0, TO_CHAR);
        return;
    }

    cmd = arguments.getOneArgument( );
    cmd.stripWhiteSpace( );
    
    ed = descFind( wall, wall->getName( ) );

    if (cmd == "+") {
        if (!ed) 
            ed = descAdd( wall, wall->getName( ) );
            
        lineAdd( ed, arguments );
        oldact("Ты выцарапываешь $O5 надпись на $o6.", ch, wall, nail, TO_CHAR );
        oldact("$c1 выцарапывает $O5 надпись на $o6.", ch, wall, nail, TO_ROOM );
    }
    else if (cmd == "-" && ch->is_immortal( )) {
        if (ed && lineDel( ed )) {
            act("Ты отшкрябываешь последнюю строчку с %3$O2.", ch, wall, 0, TO_CHAR );
            oldact("$c1 скребет $o4.", ch, wall, 0, TO_ROOM );
        }
        else
            act("%3$^O1 девственно чист(а) - удалять нечего.", ch, wall, 0, TO_CHAR );
    }
    else if (cmd == "clear" && ch->is_immortal( )) {
        descFree( wall, wall->getName( ) );
        act("Ты тщательно отшкрябываешь все надписи с %3$O2.", ch, wall, 0, TO_CHAR );
        oldact("$c1 тщательно отшкрябывает все надписи с $o2.", ch, wall, 0, TO_ROOM );
    }                
    else
        usage( ch );
}

void CWrite::writeOnPaper( Character *ch, Object *paper, DLString &arguments )
{
    EXTRA_DESCR_DATA *ed;
    DLString cmd, keyword;

    if (paper->item_type != ITEM_PARCHMENT) {
        ch->pecho("Это не пергамент.");
        return;
    }

    if (arguments.empty( )) {
        act("Что именно ты хочешь записать на %3$O4?", ch, paper, 0, TO_CHAR);
        return;
    }
    
    keyword = arguments.getOneArgument( );
    keyword.stripWhiteSpace( );

    if (keyword == "face")
        keyword = paper->getName( );

    cmd = arguments.getOneArgument( );
    cmd.stripWhiteSpace( );
    
    if (cmd != "+" && cmd != "-" && cmd != "clear") {
        usage( ch );
        return;
    }

    if (cmd == "clear") {
        descFree( paper, keyword );
        
        if (keyword == paper->getName( ))
            act("Ты стираешь все надписи с лицевой стороны %3$O2.", ch, paper, 0, TO_CHAR );
        else
            oldact("Ты стираешь с $o2 все, что касается '$T'.", ch, paper, keyword.c_str( ), TO_CHAR );

        return;
    }
    
    ed = descFind( paper, keyword );

    if (cmd == "+") {
        if (!ed) 
            ed = descAdd( paper, keyword );

        lineAdd( ed, arguments );
        
        if (keyword == paper->getName( ))
            act("Ты делаешь запись на %3$O6.", ch, paper, 0, TO_CHAR );
        else
            oldact("Ты делаешь запись на $o6 в раздел '$T'", ch, paper, keyword.c_str( ), TO_CHAR );
    }
    else if (cmd == "-") {
        if (ed && lineDel( ed ))
            if (keyword == paper->getName( ))
                act("Ты удаляешь последнюю строку с %3$O2.", ch, paper, 0, TO_CHAR );
            else
                oldact("Ты удаляешь последнюю строку с $o2 в разделе '$T'.", ch, paper, keyword.c_str( ), TO_CHAR );
        else
            act("Удалять с %3$O2 больше нечего.", ch, paper, 0, TO_CHAR );
    }
}

EXTRA_DESCR_DATA * CWrite::descFind( Object *obj, const DLString &name )
{
    char buf[MAX_STRING_LENGTH];
    EXTRA_DESCR_DATA *ed;
    
    strcpy( buf, name.c_str( ) );
    
    for (ed = obj->extra_descr; ed; ed = ed->next)
        if (is_name( buf, ed->keyword ) )
            break;

    return ed;
}

EXTRA_DESCR_DATA * CWrite::descAdd( Object *obj, const DLString &names )
{
    EXTRA_DESCR_DATA *ed;

    ed = new_extra_descr( );
    ed->keyword = str_dup( names.c_str( ) );
    ed->description = str_dup( "" );
    ed->next = obj->extra_descr;
    obj->extra_descr = ed;

    return ed;
}

void CWrite::descFree( Object *obj, const DLString &keyword ) 
{
    char buf[MAX_STRING_LENGTH];
    EXTRA_DESCR_DATA *ed, *ped = NULL;
    
    strcpy( buf, keyword.c_str( ) );
    
    for (ed = obj->extra_descr; ed; ed = ed->next) {
        if (is_name( buf, ed->keyword ) )
            break;

        ped = ed;
    }
    
    if (!ed)
        return;

    if (!ped)
        obj->extra_descr = ed->next;
    else
        ped->next = ed->next;

    free_extra_descr(ed);
}

void CWrite::lineAdd( EXTRA_DESCR_DATA *ed, const DLString &arg )
{
    ostringstream buf;

    if (ed->description)
        buf << ed->description;

    buf << arg << endl;
    free_string( ed->description );
    ed->description = str_dup( buf.str( ).c_str( ) );
}

bool CWrite::lineDel( EXTRA_DESCR_DATA *ed )
{
    DLString buf;
    DLString::size_type i1, i2;

    if (!ed->description)
        return false;
    
    buf = ed->description;
    if (buf.empty( ))
        return false;

    i1 = buf.find_last_of( "\n" );

    if (i1 == DLString::npos || i1 == 0) {
        buf.erase( );
    }
    else {
        i2 = buf.find_last_of( "\n", i1 - 1 );
        
        if (i2 == DLString::npos) 
            buf.erase( );
        else
            buf.erase( i2 + 1 );
    }

    free_string( ed->description );
    ed->description = str_dup( buf.c_str( ) );
    return true;
}

Object * CWrite::findNail( Character *ch )
{
    Object *nail;
    
    nail = get_eq_char( ch, wear_wield );

    if (!nail)
        nail = get_eq_char( ch, wear_hold );
    if (!nail)
        nail = get_eq_char( ch, wear_second_wield );
    if (!nail)
        return NULL;

    if (is_name( "гвоздь", nail->getName( ) ) || is_name( "nail", nail->getName( ) ))
        return nail;
        
    if (nail->item_type == ITEM_WEAPON)
        switch (attack_table[nail->value3()].damage) {
        case DAM_PIERCE:
        case DAM_SLASH:
            return nail;
        }

    return NULL;
}

void CWrite::usage( Character *ch )
{
    ch->pecho("Подробнее см. 'help write'.");
}


