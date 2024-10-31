/* $Id$
 *
 * ruffina, 2004
 */
#include <algorithm>
#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "commandmanager.h"

#include "loadsave.h"
#include "merc.h"
#include "interp.h"

#include "areautils.h"
#include "security.h"
#include "olc.h"
#include "onlinecreation.h"
#include "olcstate.h"

#include "def.h"

/*
 * XMLAttributeOLC
 */
XMLVnumRange::XMLVnumRange( ) 
{
}

XMLVnumRange::XMLVnumRange( int min, int max, int s ) 
            : minVnum( min ), maxVnum( max ), security( s )
{
}

void XMLAttributeOLC::removeInterval( int a, int b )
{
    RangeList::iterator i;
    RangeList nlist;

    if (a > b)
//        a ^= b ^= a ^= b;
        swap(a, b);

    for (i = vnums.begin( ); i != vnums.end( ); i++) {
        if (a < i->minVnum && b > i->maxVnum) {
            nlist.push_back( *i );
        }
        else {
            if (b < i->maxVnum)
                nlist.push_back( XMLVnumRange( b + 1, i->maxVnum, i->security ) );
            if (a > i->minVnum) 
                nlist.push_back( XMLVnumRange( i->minVnum, a - 1, i->security ) );
        }
    }
    
    vnums = nlist;
}

bool XMLAttributeOLC::isOverlapping( int a, int b )
{
    RangeList::iterator i;
    
    if (a > b)
        swap(a, b);

    for (i = vnums.begin( ); i != vnums.end( ); i++) {
        if (a >= i->minVnum && a <= i->maxVnum)
            return true;
        if (b >= i->minVnum && b <= i->maxVnum)
            return true;
    }

    return false;
}


bool XMLAttributeOLC::handle( const WebEditorSaveArguments &args )
{
    if (saveCommand.empty())
        return false;

    OLCState::Pointer state = OLCState::getOLCState(args.pch->desc);
    if (!state) {
        // Got out of OLC while in webedit.
        saveCommand.clear();
        return false;
    }

    // Copy webedit result to the attribute that is used by all 'paste' commands.
    args.pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[0].split(args.text);

    // Run '<cmd> paste' whatever.
    state->handle(args.pch->desc, const_cast<char *>(saveCommand.c_str()));

    saveCommand.clear();
    return true;
}

/*
 * security commands and functions
 */
CMD(security, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Set char security.")
{
    char buf[MAX_STRING_LENGTH];
    PCharacter *victim;
    
    if(ch->getPC()->getSecurity() < 100) {
        ch->pecho("Это не для тебя.");
        return;
    }
    argument = one_argument(argument, buf);
    
    if(!*buf) {
        ch->pecho("Usage: security <player> <#security level>");
        return;
    }
    victim = get_player_world(ch, buf);

    if(!victim) {
        ch->pecho("Char not found.");
        return;
    }

    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) {
        ch->pecho("Security must be a number.");
        return;
    }
    
    victim->setSecurity(atoi(buf));
    
    ch->pecho("Security set.");
}

CMD(olcvnum, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Set/Show/Remove allowed vnum ranges for char.")
{
    PCMemoryInterface *victim;
    XMLAttributeOLC::Pointer attr;
    DLString arg1, arg2, arg3, arg4, arg5;
    DLString arguments = argument;
    
    if(ch->getPC( )->getSecurity() < 100) {
        ch->pecho("Это не для тебя.");
        return;
    }
    
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );
    
    if (!arg1.empty( )) {
        victim = PCharacterManager::find( arg1 );

        if (!victim) {
            ch->pecho("Char not found, misspelled name?");
            return;
        }

        if (arg2 == "show") {
            std::basic_ostringstream<char> buf;

            attr = victim->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );
            
            if (!attr) {
                buf << victim->getName( ) << " doesnt own any vnum range" << endl;
            }
            else {
                XMLAttributeOLC::RangeList::iterator i;
                
                for (i = attr->vnums.begin( ); i != attr->vnums.end( ); i++)
                    buf << "Vnums {W" << i->minVnum << "{x - {W" << i->maxVnum 
                        << "{x, security {W" << i->security << "{x" << endl;
            }
            
            ch->send_to( buf );
            return;
        }
        if (arg2 == "del") {
            arg3 = arguments.getOneArgument( );
            arg4 = arguments.getOneArgument( );

            attr = victim->getAttributes( ).findAttr<XMLAttributeOLC>( "olc" );

            if (attr) {
                try {
                    attr->removeInterval( arg3.toInt( ), arg4.toInt( ) );
                } catch (const ExceptionBadType &e) {
                    ch->pecho("Vnum ranges must be numbers.");
                    return;
                }
                
                ch->pecho("Ok.");
            }
            else
                ch->pecho("No vnum ranges found.");
            
            return;
        }
        if (arg2 == "set") {
            int a, b;

            arg3 = arguments.getOneArgument( );
            arg4 = arguments.getOneArgument( );
            arg5 = arguments.getOneArgument( );
    
            try {
                attr = victim->getAttributes( ).getAttr<XMLAttributeOLC>( "olc" );
                a = arg3.toInt( );
                b = arg4.toInt( );

                if (attr->isOverlapping( a, b )) {
                    ch->pecho("Vnums overlap existing ranges. Try olcvnum <name> show.");
                    return;
                }
                
                attr->vnums.push_back( XMLVnumRange( a, b, arg5.toInt( ) ) );
                PCharacterManager::saveMemory( victim );
                ch->pecho("Vnum range granted.");
                return;
                
            } catch (const ExceptionBadType& e) {
            }
            return;
        }

        // Create a <playername.are> area with 100 vnums and grant permissions to the player.
        if (arg_is(arg2, "create")) {
            DLString filename = victim->getName().toLower() + ".are";
            AreaIndexData *area = get_area_index(filename);
            if (area) {
                ch->pecho("Зона '%s' (%s) для этого персонажа уже существует. Используй {y{hcaedit %d{x для редактирования.",
                            area->name, area->area_file->file_name.c_str(), area->vnum);
                return;
            }

            area = AreaUtils::createFor(victim);
            ch->pecho("Создана зона '%s' (%s), диапазон внумов %d-%d. Используй {y{hcaedit %d{x для редактирования.",
                        area->name, area->area_file->file_name.c_str(), area->min_vnum, area->max_vnum, area->vnum);

            attr = victim->getAttributes( ).getAttr<XMLAttributeOLC>( "olc" );
            attr->vnums.push_back(XMLVnumRange(area->min_vnum, area->max_vnum, 9));
            PCharacterManager::saveMemory(victim);
            return;
        }
    } 

    ch->pecho( "Usage: olcvnum <player> set <min vnum> <max vnum> <security level>\r\n" 
                 "       olcvnum <player> show\r\n" 
                 "       olcvnum <player> create\r\n"
                 "       olcvnum <player> del <min vnum> <max vnum>" );

}
