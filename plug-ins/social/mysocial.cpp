/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>

#include "class.h"
#include "regexp.h"

#include "mysocial.h"
#include "pcharacter.h"
#include "comm.h"
#include "arg_utils.h"
#include "act.h"
#include "merc.h"


/*----------------------------------------------------------------------
 * 'mysocial' command
 *---------------------------------------------------------------------*/
COMMAND(MySocial, "mysocial")
{
    DLString arg = constArguments, cmd, name;
    XMLAttributeCustomSocials::Pointer attr;

    if (ch->is_npc( ))
        return;
        
    cmd = arg.getOneArgument( );
    name = arg.getOneArgument( );
    
    if (cmd.empty( )) {
        usage( ch );
        return;
    }

    attr = ch->getPC( )->getAttributes( ).getAttr<XMLAttributeCustomSocials>( "socials" );

    if (arg_is_list( cmd ))
        doList( ch, attr );
    else if (arg_is_help( cmd ))
        usage( ch );
    else if (name.empty( )) 
        ch->pecho( "Укажи имя социала." );
    else if (arg_oneof( cmd, "del", "удалить" ))
        doDelete( ch, attr, name );
    else if (arg_is_show( cmd ))
        doShow( ch, attr, name );
    else if (arg.empty( )) 
        ch->pecho( "Укажи, какой вид сообщения изменить и как. Смотри '{lRмойсоц ?{lEmysoc help{lx'." );
    else {
        CustomSocial::Pointer social;
        XMLAttributeCustomSocials::iterator i = attr->find( name );
        
        if (i == attr->end( )) {
            social.construct( );
            (**attr)[name] = social;
        }
        else
            social = i->second;

        social->setName( name );
        
        if (arg_oneof( cmd, "rus", "рус" ))
        {
            social->setRussianName( arg );
        }
        else if (arg_oneof( cmd, "noarg_other", "безцели_другие" ))
        {
            if (hasMeOnly( ch, arg )) 
                social->setNoargOther( arg );
            else
                return;
        }
        else if (arg_oneof( cmd, "noarg_me", "безцели_я" ))
        {
            if (hasNoVariables( ch, arg ))
                social->setNoargMe( arg );
            else
                return;
        }
        else if (arg_oneof( cmd, "auto_me", "насебя_я" ))
        {
            if (hasNoVariables( ch, arg ))
                social->setAutoMe( arg );
            else
                return;
        }
        else if (arg_oneof( cmd, "auto_other", "насебя_другие" ))
        {
            if (hasMeOnly( ch, arg ))
                social->setAutoOther( arg );
            else
                return;
        }
        else if (arg_oneof( cmd, "arg_victim", "нацель_жертва" ))
        {
            if (hasMeOnly( ch, arg ))
                social->setArgVictim( arg );
            else
                return;
        }
        else if (arg_oneof( cmd, "arg_me", "нацель_я" ))
        {
            if (hasVictOnly( ch, arg ))
                social->setArgMe( arg );
            else
                return;
        }
        else if (arg_oneof( cmd, "arg_other", "нацель_другие" ))
        {
            if (hasBoth( ch, arg ))
                social->setArgOther( arg );
            else
                return;
        }
        else {
            ch->pecho("Нет такого вида сообщений. Смотри '{lRмойсоц ?{lEmysoc help{lx'.");
            return;
        }

        ch->pecho("Ok.");
    }
}

bool MySocial::hasMeOnly( Character *ch,  const DLString &arg )
{
    static RegExp pat( "^\\$c[1-6] [^\\$]*$", true );
    
    if (!pat.match( arg )) {
        ch->pecho("Сообщение должно начинаться с твоего имени (в нужном падеже) и не содержать других переменных. Используй $c1, $c2...$c6.");
        return false;
    }
    else 
        return true;
}

bool MySocial::hasNoVariables( Character *ch, const DLString &arg )
{
    static RegExp pat( "^[^\\$]*$" );

    if (!pat.match( arg )) {
        ch->pecho("Слишком много долларов! В этом сообщении нельзя использовать переменные.");
        return false;
    }

    return true;
}

bool MySocial::hasVictOnly( Character *ch, const DLString &arg )
{
    static RegExp pat( "^.*\\$C[1-6].*$|"
                       "^[^\\$]*$", true );
    
    if (!pat.match( arg )) {
        ch->pecho("В этом сообщении может встречаться только имя жертвы в нуждом падеже ($C1, .. $C6).");
        return false;
    }

    return true;
}

bool MySocial::hasBoth( Character *ch, const DLString &arg )
{
    static RegExp pat( "^\\$c[1-6] |"
                       "^\\$c[1-6] [^\\$]*$|"
                       "^\\$c1[1-6] .*\\$C[1-6][^\\$]*$", true );
    
    if (!pat.match( arg )) {
        ch->pecho( "Сообщение должно начинаться с твоего имени в нужном падеже ($c1, .. $c6) \r\n"
                     "и кроме него может содержать еще только имя жертвы ($C1, ... $C6)." ); 
        return false;
    }
    else 
        return true;
}


void MySocial::doList( Character *ch, XMLAttributeCustomSocials::Pointer attr )
{
    XMLAttributeCustomSocials::iterator i;
    ostringstream buf;
    
    if (attr->empty( )) {
        oldact_p("Ты пока не придума$gло|л|ла ни одного собственного социала.", ch, 0, 0, TO_CHAR, POS_DEAD);
        return;
    }
    
    bool fRus = ch->getConfig( ).rucommands;

    buf << "{W----------------+--------------------------------------------------------------{x" << endl;
      
    for (i = attr->begin( ); i != attr->end( ); i++) {
        CustomSocial::Pointer c = i->second;
        
        buf << dlprintf( "{c%-14s{x | %-11s ",
                          c->getName( ).c_str( ),
                          c->getRussianName( ).c_str( ) ) << endl;

        if (!c->getNoargMe( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "безцели_я" : "noarg_me" )
                << c->getNoargMe( ) << endl;
        if (!c->getNoargOther( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "безцели_другие" : "noarg_other" )
                << c->getNoargOther( ) << endl;
        if (!c->getArgOther( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "нацель_другие" : "arg_other" )
                << c->getArgOther( ) << endl;
        if (!c->getArgMe( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "нацель_я" : "arg_me" )
                << c->getArgMe( ) << endl;
        if (!c->getArgVictim( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "нацель_жертва" : "arg_victim" )
                << c->getArgVictim( ) << endl;
        if (!c->getAutoMe( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "насебя_я" : "auto_me" )
                << c->getAutoMe( ) << endl;
        if (!c->getAutoOther( ).empty( ))
            buf << dlprintf( "%-14s | ", fRus ? "насебя_другие" : "auto_other" )
                << c->getAutoOther( ) << endl;

        buf << "{W----------------+--------------------------------------------------------------{x" << endl;
    }

    page_to_char( buf.str( ).c_str( ), ch );
}

void MySocial::doDelete( Character *ch, XMLAttributeCustomSocials::Pointer attr, const DLString &name )
{
    XMLAttributeCustomSocials::iterator i;

    i = attr->find( name );

    if (attr->end( ) == i)
        ch->pecho("Социал с таким именем не найден.");
    else {
        attr->erase( i );
        ch->pecho("Ok.");
    }
}

void MySocial::doShow( Character *ch, XMLAttributeCustomSocials::Pointer attr, const DLString &name )
{
    ostringstream buf;
    CustomSocial::Pointer c;
    XMLAttributeCustomSocials::iterator i = attr->find( name );
    
    if (i == attr->end( )) {
        ch->pecho("Социал с таким именем не найден.");
        return;
    }

    c = i->second;

    buf << "{c" << c->getName( ) << "{x";

    if (!c->getRussianName( ).empty( ))
        buf << "({c" << c->getRussianName( ) << "{x)";

    buf << endl;

    buf << "{cПри использовании без аргумента{x: " << endl
        << "Ты увидишь ({lRбезцели_я{lEnoarg_me{lx):  " << c->getNoargMe( ) << endl  
        << "Окружающие увидят ({lRбезцели_другие{lEnoarg_other{lx):  " << c->getNoargOther( ) << endl 
        << endl
        << "{cПри использовании на кого-то{x: " << endl
        << "Ты увидишь ({lRнацель_я{lEarg_me{lx):  " << c->getArgMe( ) << endl
        << "Жертва увидит ({lRнацель_жертва{lEarg_victim{lx):  " << c->getArgVictim( ) << endl 
        << "Все остальные увидят ({lRнацель_другие{lEarg_other{lx):  " << c->getArgOther( ) << endl 
        << endl
        << "{cПри использовании на самого себя{x: " << endl
        << "Ты увидишь ({lRнасебя_я{lEauto_me{lx):  " << c->getAutoMe( ) << endl 
        << "Окружающие увидят ({lRнасебя_другие{lEauto_other{lx):  " << c->getAutoOther( ) << endl;

    page_to_char( buf.str( ).c_str( ), ch );
}


void MySocial::usage( Character *ch )
{
    ostringstream buf;
    
    buf << "{W{lRмойсоциал список{lEmysocial list{lx{w" << endl
        << "      - показать список социалов" << endl
        << "{W{lRмойсоциал показать{lEmysocial show{lx{w <название>" << endl
        << "      - показать подробности социала" << endl
        << "{W{lRмойсоциал удалить{lEmysocial del{lx{w <название>" << endl
        << "      - удалить социал" << endl
        << "{W{lRмойсоциал рус{lEmysocial rus{lx{w <название> <синоним>" << endl
        << "      - присвоить русский синоним" << endl
        << "{W{lRмойсоциал безцели_другие|безцели_я{lEmysocial noarg_other|noarg_me{lx{w <название> <строка>" << endl
        << "      - задать социал, который используется без указания цели" << endl
        << "{W{lRмойсоциал насебя_другие|насебя_я{lEmysocial auto_other|auto_me{lx{w <название> <строка>" << endl
        << "      - задать социал, который используется на самого себя" << endl
        << "{W{lRмойсоциал нацель_другие|нацель_я|нацель_жертва{lEmysocial arg_other|arg_me|arg_victim{lx{w <название> <строка>" << endl
        << "      - задать социал, который используется на кого-то другого" << endl
        << endl
        << "Подробности смотри в '{lR? мойсоциал{lEhelp mysocial{lx'." << endl;

    ch->send_to( buf );
}

