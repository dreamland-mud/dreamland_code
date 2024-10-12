/* $Id: lover.cpp,v 1.1.2.10.10.3 2009/09/06 21:48:28 rufina Exp $
 * ruffina, 2003
 */

#include "lover.h"
#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"
#include "xmllovers.h"

#include "logstream.h"
#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"

#include "arg_utils.h"
#include "loadsave.h"
#include "merc.h"
#include "act.h"
#include "def.h"

const DLString Lover::XMLAttributeLoverString = "XMLAttributeLovers";

COMMAND(Lover, "lover")
{
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    
    if (ch->is_npc()) {
        ch->pecho("Тебе нельзя.");
        return;
    }

    if (IS_CHARMED(ch)) {
        oldact("... но сердцу не прикажешь.", ch, 0, 0, TO_CHAR);  
        oldact("$c1 ухмыляется - сердцу не прикажешь.", ch, 0, ch->master, TO_VICT);
        return;
    }
    
    if (cmd.empty( ))
        usage( ch );
    else if (arg_is_list( cmd ))
        list( ch, arguments );
    else if (arg_is(cmd, "add"))
        add( ch, arguments );
    else if (arg_is(cmd, "del"))
        del( ch, arguments );
    else 
        usage( ch );
        
}

void Lover::list( Character* ch, DLString arguments) 
{
    std::basic_ostringstream<char> str;
    XMLAttributes *attributes;
    XMLAttributeLovers::Pointer pointer;

    attributes = &ch->getPC( )->getAttributes( );
    XMLAttributes::iterator ipos = attributes->find( "lovers" );
        
    if (ipos == attributes->end( ) ||
        (pointer = ipos->second.getDynamicPointer<XMLAttributeLovers>( ))->lovers.empty( )) 
    {
        str << "Ты никого не любишь." << endl;
    }
    else {
        str << "{W- {RЛюбовники{W -" << endl;
        
        for (XMLLovers::iterator loverpos = pointer->lovers.begin( );
             loverpos != pointer->lovers.end( );
             loverpos++) 
        {
            str << loverpos->first << endl;
        }
        
        str << "{x";
   }
    
    ch->send_to( str );
}

void Lover::add( Character* ch, DLString arguments)
{
    DLString name = arguments.getOneArgument();
    std::basic_ostringstream<char> str;

    if (PCMemoryInterface* pci = PCharacterManager::find( name )) {
        ch->getPC( )->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->
                            lovers.put( pci->getName( ) );

        str << "Ты отдаешь свое сердце "<<  pci->getName( ) << "." << endl;
    }
    else {
        str << "Таких нет." << endl;
    }

    ch->send_to( str );
}

void Lover::del( Character* ch, DLString arguments)
{
    DLString name = arguments.getOneArgument( );
    std::basic_ostringstream<char> str;
    XMLAttributes *attributes;
    XMLAttributeLovers::Pointer pointer;

    attributes = &ch->getPC( )->getAttributes( );
    XMLAttributes::iterator ipos = attributes->find( "lovers" );
        
    if (ipos == attributes->end( ) ||
        (pointer = ipos->second.getDynamicPointer<XMLAttributeLovers>( ))->lovers.empty( )) 
    {        
        str << "Но твой список любовников и так пуст!" << endl;
    }
    else {
        name.toLower( ).upperFirstCharacter( );

        if (pointer->lovers.isPresent( name )) {
            XMLAttributeMarriage::Pointer mattr;
            
            mattr = attributes->findAttr<XMLAttributeMarriage>( "marriage" );

            if (mattr && mattr->spouse.getValue( ) == name) {
                str << "Только развод поможет тебе." << endl;
            } else {
                pointer->lovers.erase( name );
                str << "Твое сердце больше не трепещет при виде " << name << "." << endl;
            }
        }
        else 
            str << "Ты и так не пылаешь страстью к " << name << ". " << endl;        
    }
    
    ch->send_to( str );
}

void Lover::usage( Character* ch ) 
{
    std::basic_ostringstream<char> str;
    
    str << "любовники список - просмотр списка любовников" << endl
        << "любовники добавить <жертва> - отдать свое сердце кому-то" << endl
        << "любовники удалить <жертва> - удалить кого-то из списка любовников" << endl;

    ch->send_to( str );
}

bool mlove_accepts(Character *ch, Character *victim)
{
    if (victim->is_npc() || ch->is_npc())
        return false;


    XMLAttributeLovers::Pointer attr =
        victim->getPC()->getAttributes().findAttr<XMLAttributeLovers>("lovers");
    
    return attr && attr->lovers.isPresent(ch->getName()); 
}

