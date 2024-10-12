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
    else if (arg_is(cmd, "del"))
        doDelete( ch, attr, name );
    else if (arg_is_show( cmd ))
        doShow( ch, attr, name );
    else if (arg.empty( )) 
        ch->pecho( "Укажи, какой вид сообщения изменить и как. Смотри 'мойсоц ?'." );
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
        
        if (arg_is(cmd, "ru"))
        {
            social->setRussianName( arg );
        }
        else if (arg_is(cmd, "noarg_other"))
        {
            if (hasMeOnly( ch, arg )) 
                social->setNoargOther( arg );
            else
                return;
        }
        else if (arg_is(cmd, "noarg_me"))
        {
            if (hasNoVariables( ch, arg ))
                social->setNoargMe( arg );
            else
                return;
        }
        else if (arg_is(cmd, "auto_me"))
        {
            if (hasNoVariables( ch, arg ))
                social->setAutoMe( arg );
            else
                return;
        }
        else if (arg_is(cmd, "auto_other"))
        {
            if (hasMeOnly( ch, arg ))
                social->setAutoOther( arg );
            else
                return;
        }
        else if (arg_is(cmd, "arg_victim"))
        {
            if (hasMeOnly( ch, arg ))
                social->setArgVictim( arg );
            else
                return;
        }
        else if (arg_is(cmd, "arg_me"))
        {
            if (hasVictOnly( ch, arg ))
                social->setArgMe( arg );
            else
                return;
        }
        else if (arg_is(cmd, "arg_other"))
        {
            if (hasBoth( ch, arg ))
                social->setArgOther( arg );
            else
                return;
        }
        else {
            ch->pecho("Нет такого вида сообщений. Смотри 'мойсоц ?'.");
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
        
        buf << fmt(0, "{c%-14s{x | %-11s ",
                          c->getName( ).c_str( ),
                          c->getRussianName( ).c_str( ) ) << endl;

        if (!c->getNoargMe( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "безцели_я" : "noarg_me" )
                << c->getNoargMe( ) << endl;
        if (!c->getNoargOther( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "безцели_другие" : "noarg_other" )
                << c->getNoargOther( ) << endl;
        if (!c->getArgOther( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "нацель_другие" : "arg_other" )
                << c->getArgOther( ) << endl;
        if (!c->getArgMe( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "нацель_я" : "arg_me" )
                << c->getArgMe( ) << endl;
        if (!c->getArgVictim( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "нацель_жертва" : "arg_victim" )
                << c->getArgVictim( ) << endl;
        if (!c->getAutoMe( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "насебя_я" : "auto_me" )
                << c->getAutoMe( ) << endl;
        if (!c->getAutoOther( ).empty( ))
            buf << fmt(0, "%-14s | ", fRus ? "насебя_другие" : "auto_other" )
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
        << "Ты увидишь (безцели_я):  " << c->getNoargMe( ) << endl  
        << "Окружающие увидят (безцели_другие):  " << c->getNoargOther( ) << endl 
        << endl
        << "{cПри использовании на кого-то{x: " << endl
        << "Ты увидишь (нацель_я):  " << c->getArgMe( ) << endl
        << "Жертва увидит (нацель_жертва):  " << c->getArgVictim( ) << endl 
        << "Все остальные увидят (нацель_другие):  " << c->getArgOther( ) << endl 
        << endl
        << "{cПри использовании на самого себя{x: " << endl
        << "Ты увидишь (насебя_я):  " << c->getAutoMe( ) << endl 
        << "Окружающие увидят (насебя_другие):  " << c->getAutoOther( ) << endl;

    page_to_char( buf.str( ).c_str( ), ch );
}


void MySocial::usage( Character *ch )
{
    ostringstream buf;
    
    buf << "{Wмойсоциал список{w" << endl
        << "      - показать список социалов" << endl
        << "{Wмойсоциал показать{w <название>" << endl
        << "      - показать подробности социала" << endl
        << "{Wмойсоциал удалить{w <название>" << endl
        << "      - удалить социал" << endl
        << "{Wмойсоциал рус{w <название> <синоним>" << endl
        << "      - присвоить русский синоним" << endl
        << "{Wмойсоциал безцели_другие|безцели_я{w <название> <строка>" << endl
        << "      - задать социал, который используется без указания цели" << endl
        << "{Wмойсоциал насебя_другие|насебя_я{w <название> <строка>" << endl
        << "      - задать социал, который используется на самого себя" << endl
        << "{Wмойсоциал нацель_другие|нацель_я|нацель_жертва{w <название> <строка>" << endl
        << "      - задать социал, который используется на кого-то другого" << endl
        << endl
        << "Подробности смотри в '? мойсоциал'." << endl;

    ch->send_to( buf );
}

