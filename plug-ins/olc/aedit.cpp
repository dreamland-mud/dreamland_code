/* $Id$
 *
 * ruffina, 2004
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <config.h>

#include <character.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "room.h"

#include "aedit.h"

#include "olc.h"

#include "merc.h"
#include "update_areas.h"
#include "interp.h"
#include "mercdb.h"

#include "security.h"

#include "def.h"

OLC_STATE(OLCStateArea);

OLCStateArea::OLCStateArea() : vnum( -1 ), area_flag(0, &area_flags)
{
    /*fromXML will!*/
}

OLCStateArea::OLCStateArea(AREA_DATA *original) : area_flag(0, &area_flags)
{
    if (original) {
        if(original->area_file->file_name)
            file_name = original->area_file->file_name;
        if(original->name)
            name      = original->name;
        if(original->name)
            credits   = original->credits;
        age       = original->age;
        nplayer   = original->nplayer;
        low_range = original->low_range;
        high_range= original->high_range;
        min_vnum  = original->min_vnum;
        max_vnum  = original->max_vnum;
        if(original->resetmsg)
            resetmsg  = original->resetmsg;
        area_flag.setValue( original->area_flag );
        security  = original->security;
        if(original->authors)
            authors = original->authors;
        if(original->altname)
            altname = original->altname;
        if (original->translator)
            translator = original->translator;
        if (original->speedwalk)
            speedwalk = original->speedwalk;
        vnum      = original->vnum;

        if (original->behavior) 
            try {
                ostringstream ostr;
                original->behavior.toStream( ostr );
                behavior.assign( ostr.str( ) );
            } catch (const ExceptionXMLError &e) {
                LogStream::sendError( ) << e.what( ) << endl;
            }
    }
    else {
        top_area++;
        vnum = top_area - 1;

        name = "New area";
        area_flag.setBit( AREA_ADDED );
        security = 9;
        authors = "None";
        translator = "None";
        speedwalk = "";
        altname = "";
        min_vnum = 0;
        max_vnum = 0;
        low_range = 0;
        high_range = 0;
        age = 0;
        nplayer = 0;
        credits = "None";
        resetmsg = "";
        file_name.setValue( DLString( "area" ) + DLString( vnum ) + ".are" );
    }
}

OLCStateArea::~OLCStateArea() 
{
}

void OLCStateArea::commit() 
{
    AREA_DATA *original;
    
    for (original = area_first; original; original = original->next)
        if(original->vnum == vnum)
            break;
    
    if(!original) {
        original = new AREA_DATA;
        original->next = NULL;
        original->area_file = new_area_file( file_name.getValue( ).c_str( ) );
        original->area_file->area = original;
        original->empty = true;
        original->count = 0;

        area_last->next = original;
        area_last = original;
    }
    else {
        free_string(original->area_file->file_name);
        original->area_file->file_name = str_dup( file_name.getValue( ).c_str( ) );
        free_string(original->name);
        free_string(original->credits);
        free_string(original->resetmsg);
        free_string(original->authors);
        free_string(original->altname);
        free_string(original->translator);
        free_string(original->speedwalk);
    }

    original->name      = str_dup( name.getValue( ).c_str( ) );
    original->credits   = str_dup( credits.getValue( ).c_str( ) );
    original->age       = age;
    original->nplayer   = nplayer;
    original->low_range = low_range;
    original->high_range= high_range;
    original->min_vnum  = min_vnum;
    original->max_vnum  = max_vnum;
    original->resetmsg  = str_dup( resetmsg.getValue( ).c_str( ) );
    original->area_flag = area_flag;
    original->security  = security;
    original->authors = str_dup( authors.getValue( ).c_str( ) );
    original->altname = str_dup( altname.getValue( ).c_str( ) );
    original->translator = str_dup( translator.getValue( ).c_str( ) );
    original->speedwalk = str_dup( speedwalk.getValue( ).c_str( ) );
    original->vnum = vnum;

    if (!behavior.empty( )) 
        try {
            istringstream istr( behavior );
            original->behavior.fromStream( istr );
            original->behavior->setArea( original );
        } catch (const Exception &e) {
            LogStream::sendError( ) << e.what( ) << endl;
        }
}

void OLCStateArea::statePrompt(Descriptor *d) 
{
    d->send( "Editing area> " );
}

bool OLCStateArea::checkOverlap(int lower, int upper)
{
    AREA_DATA *pArea;

    for (pArea = area_first; pArea; pArea = pArea->next) {
        if(pArea->vnum == vnum)
            continue;
            
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper))
            return false;

        if ((lower <= pArea->max_vnum && pArea->max_vnum <= upper))
            return false;
    }
    return true;
}

void OLCStateArea::changed( PCharacter *ch )
{
    area_flag.setValue( area_flag.getValue( ) | AREA_CHANGED);
}

// Area Editor Functions.
AEDIT(show)
{
    ptc(ch, "Name:    [%5d] %s\n\r", vnum.getValue( ), name.getValue( ).c_str( ));
    ptc(ch, "File:    [%s]\n\r", file_name.getValue( ).c_str( ));
    ptc(ch, "Vnums:   [%u-%u]\n\r", min_vnum.getValue( ), max_vnum.getValue( ));
    ptc(ch, "Levels:  [%u-%u]\n\r", low_range.getValue( ), high_range.getValue( ));
    ptc(ch, "Age:     [%d]\n\r", age.getValue( ));
    ptc(ch, "Players: [%d]\n\r", nplayer.getValue( ));
    ptc(ch, "Security:[%d]\n\r", security.getValue( ));
    ptc(ch, "Authors :[%s]\n\r", authors.getValue( ).c_str( ));
    ptc(ch, "Credits :[%s]\n\r", credits.getValue( ).c_str( ));
    ptc(ch, "Translator:[%s]\n\r", translator.getValue( ).c_str( ));
    ptc(ch, "Speedwalk:[%s]\n\r", speedwalk.getValue( ).c_str( ));
    ptc(ch, "Resetmsg:[%s]\n\r", resetmsg.getValue( ).c_str( ));
    ptc(ch, "Flags:   [%s]\n\r", area_flags.names(area_flag).c_str());

    if (!behavior.empty( ))
        ptc(ch, "Behavior:\r\n%s", behavior.c_str( ));
    
    return false;
}

AEDIT(reset)
{
    AREA_DATA *original;

    for (original = area_first; original; original = original->next)
        if(original->vnum == vnum) {
            reset_area(original);
            stc("Ария сброшена.\n\r", ch);
            return false;
        }

    stc("Создание арии не завершено - нечего сбрасывать.\n\r", ch);
    return false;
}

AEDIT(create)
{
    OLCStateArea::Pointer ae(NEW, (AREA_DATA *)NULL);
    ae->attach(ch);

    stc("Aрия создана.\n\r", ch);
    return false;
}

AEDIT(name)
{
    if (!*argument) {
        stc("Синтаксис:   name [$имя]\n\r", ch);
        return false;
    }

    name = argument;

    stc("Имя присвоено.\n\r", ch);
    return true;
}


AEDIT(credits)
{
    if (!*argument) {
        stc("Синтаксис:   credits [$credits]\n\r", ch);
        return false;
    }

    credits = argument;

    stc("Копирайт установлен.\n\r", ch);
    return true;
}

AEDIT(message)
{        
    if (!*argument) {
        stc("Синтаксис:   message [resetmsg]\n\r", ch);
        return false;
    }

    resetmsg = argument;

    stc("Resetmsg установлен.\n\r", ch);
    return true;
}

AEDIT(flags)
{
    bitstring_t value;

    if (!*argument) {
        stc("Синтаксис:   flag [$areaflags]\n\r", ch);
        return false;
    }

    value = area_flags.bitstring( argument );

    if (value == NO_FLAG) {
        stc("No such area flag.\n\r", ch);
        return false;
    }
    
    area_flag.toggleBit( value );
    area_flag.setBit( AREA_CHANGED );
    stc("Flag toggled.\n\r", ch);
    return true;
}

AEDIT(file)
{
    char file[MAX_STRING_LENGTH];
    int i, length;

    if (ch->getSecurity( ) < 10) {
        stc("Недостаточно прав для изменения имени файла арии.\r\n", ch);
        return false;
    }

    one_argument(argument, file);        /* Forces Lowercase */

    if (!*argument) {
        stc("Синтаксис:  filename [$имя_file]\n\r", ch);
        return false;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen(argument);
    if (length > 64) {
        stc("No more than eight characters allowed.\n\r", ch);
        return false;
    }

    /*
     * Allow only letters and numbers.
     */
    for (i = 0; i < length; i++) {
        if (!isalnum(file[i])) {
            stc("Only letters and numbers are valid.\n\r", ch);
            return false;
        }
    }

    strcat(file, ".are");
    file_name = file;

    stc("Filename set.\n\r", ch);
    return true;
}

AEDIT(age)
{
    char age[MAX_STRING_LENGTH];

    one_argument(argument, age);

    if (!is_number(age) || !*age) {
        stc("Syntax:  age [#xage]\n\r", ch);
        return false;
    }

    this->age = atoi(age);

    stc("Age set.\n\r", ch);
    return true;
}


AEDIT(security)
{
    char sec[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int value;

    one_argument(argument, sec);

    if (!is_number(sec) || !*sec) {
        stc("Syntax:  security [#xlevel]\n\r", ch);
        return false;
    }

    value = atoi(sec);

    if (value > ch->getSecurity( ) || value < 0) {
        if (ch->getSecurity() != 0) {
            sprintf(buf, "Security is 0-%d.\n\r", ch->getSecurity());
            stc(buf, ch);
        }
        else
            stc("Security is 0 only.\n\r", ch);
        return false;
    }

    security = value;

    stc("Security set.\n\r", ch);
    return true;
}

AEDIT(authors)
{
    if (!*argument) {
        stc("Syntax:  authors names\n\r", ch);
        stc("Syntax:  authors none\n\r", ch);
        return false;
    }

    if (!str_cmp(argument, "none"))
        authors = "none";
    else
        authors = argument;

    return true;
}

AEDIT(translator)
{
    if (!*argument) {
        stc("Syntax:  translator names\n\r", ch);
        stc("Syntax:  translator none\n\r", ch);
        return false;
    }

    if (!str_cmp(argument, "none"))
        translator = "none";
    else
        translator = argument;

    return true;
}

AEDIT(speedwalk)
{
    if (!*argument) {
        stc("Syntax:  speedwalk <run from MSM>\n\r", ch);
        stc("Syntax:  speedwalk none\n\r", ch);
        return false;
    }

    if (!str_cmp(argument, "none"))
        speedwalk = "";
    else
        speedwalk = argument;

    return true;
}
AEDIT(vnum)
{
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }
    
    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || !*lower || !is_number(upper) || !*upper) {
        stc("Syntax:  vnum [#xlower] [#xupper]\n\r", ch);
        return false;
    }

    ilower = atoi(lower);
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    min_vnum = ilower;
    max_vnum = iupper;
    stc("Lower & Upper vnums set.\n\r", ch);    
    return true;
}

AEDIT(lvnum)
{
    char lower[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }

    one_argument(argument, lower);

    if (!is_number(lower) || !*lower) {
        stc("Syntax:  min_vnum [#xlower]\n\r", ch);
        return false;
    }
    
    ilower = atoi(lower);
    iupper = max_vnum;

    if (ilower > iupper) {
        stc("OLCStateArea:  Value must be less than the max_vnum.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    min_vnum = ilower;
    stc("Lower vnum set.\n\r", ch);
    return true;
}

AEDIT(uvnum)
{
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }

    one_argument(argument, upper);

    if (!is_number(upper) || !*upper) {
        stc("Syntax:  max_vnum [#xupper]\n\r", ch);
        return false;
    }
    
    ilower = min_vnum;
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    max_vnum = iupper;
    stc("Upper vnum set.\n\r", ch);
    return true;
}

AEDIT(levels)
{
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || !*lower || !is_number(upper) || !*upper) {
        stc("Syntax:  levels [#xlower] [#xupper]\n\r", ch);
        return false;
    }
    
    ilower = atoi(lower);
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    low_range = ilower;
    high_range = iupper;
    stc("Level range set.\n\r", ch);
    return true;
}

AEDIT(behavior)
{
    if (argument[0] == '\0') {
        if(!sedit(behavior))
            return false;

        stc("Behavior set.\n\r", ch);
        return true;
    }

    stc("Syntax:  behavior\n\r", ch);
    return false;
}

AEDIT(commands)
{
    do_commands(ch);
    return false;
}

AEDIT(done) 
{
    commit();
    detach(ch);
    return true;
}

AEDIT(cancel)
{
    detach(ch);
    return false;
}

AEDIT(dump)
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}

CMD(aedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online area editor.")
{
    AREA_DATA *pArea;
    int value;
    char arg[MAX_STRING_LENGTH];

    pArea = ch->in_room->area;

    argument = one_argument(argument, arg);
    if (is_number(arg)) {
        value = atoi(arg);
        pArea = get_area_data(value);
        if (!pArea) {
            stc("That area vnum does not exist.\n\r", ch);
            return;
        }
    }
    else {
        if (!str_cmp(arg, "create")) {
            if (ch->getSecurity() < 10) {
                stc("Insuficiente seguridad para crear areas.\n\r", ch);
                return;
            }
            OLCStateArea::Pointer ae(NEW, (AREA_DATA *)NULL);
            ae->attach(ch);
            stc("Ария создана.\r\n", ch);
            return;
        }
    }

    if (!OLCState::can_edit(ch, pArea)) {
        stc("У тебя недостаточно прав для редактирования арий.\n\r", ch);
        return;
    }

    OLCStateArea::Pointer ae(NEW, pArea);
    ae->attach(ch);
}

