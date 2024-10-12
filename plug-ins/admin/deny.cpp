/* $Id: deny.cpp,v 1.1.2.7.6.3 2008/03/21 22:16:43 rufina Exp $
 *
 * ruffina, 2004
 */

#include "deny.h"

#include "class.h"

#include "commonattributes.h"
#include "pcharacter.h"
#include "pcharactermanager.h"

#include "wiznet.h"
#include "interp.h"
#include "act.h"
#include "merc.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * 'deny' command
 *---------------------------------------------------------------------------*/
void Deny::action(const DLString &constArguments, ostringstream &buf)
{
    PCMemoryInterface *pcm;
    DLString name;
    DLString argument = constArguments;
    
    if (argument.empty( )) {
        doUsage( buf );
        return;
    }
    
    name = argument.getOneArgument( );
    pcm = PCharacterManager::find( name );
    
    if (!pcm) {
        buf << fmt(0,  "Victim '%s' not found, misspelled name?", name.c_str( )) << endl;
        return;
    }
    
    try {
        if (argument.empty( )) 
            doShow( pcm, buf );
        else if (argument == "off") 
            doRemove( pcm, buf );
        else
            doPlace( pcm, argument, buf );
            
    } catch (Exception e) {
        buf << e.what() << endl;
    }
}

COMMAND(Deny, "deny")
{
    ostringstream buf;

    if (!ch->isCoder())
        return;

    Deny::action(constArguments, buf);
    ch->send_to(buf);
}

void Deny::doShow( PCMemoryInterface *pcm, ostringstream &buf )
{
    XMLAttributeDeny::Pointer attr;
    
    attr = pcm->getAttributes( ).findAttr<XMLAttributeDeny>( "deny" );
    
    if (attr) 
       buf << fmt(0, "Access for {W%s{x is denied {W%s{x by {W%s{x.\r\n",
                    pcm->getName( ).c_str( ),
                    attr->getUntilString( false ).c_str( ),
                    attr->getResponsible( ).c_str( ) );
    else
        buf << fmt(0, "Access for {W%s{x is NOT denied.\r\n", pcm->getName( ).c_str( ) );
}

void Deny::doRemove( PCMemoryInterface *pcm, ostringstream &buf )
{
    XMLAttributeDeny::Pointer attr;
    
    attr = pcm->getAttributes( ).findAttr<XMLAttributeDeny>( "deny" );

    if (attr) {
        attr->end( pcm );
        pcm->getAttributes( ).eraseAttribute( "deny" );
        PCharacterManager::saveMemory( pcm );
        
        buf << "Ok." << endl;
    }
    else
       buf << fmt(0, "Access for {W%s{x is NOT denied.\r\n", pcm->getName( ).c_str( ) );
}

void Deny::doPlace( PCMemoryInterface *pcm, const DLString & argument, ostringstream &buf )
{
    XMLAttributeDeny::Pointer attr;
    int time;
    
    if (argument == "forever")
        time = -1;
    else
        time = Date::getSecondFromString( argument );
    
    attr = pcm->getAttributes( ).getAttr<XMLAttributeDeny>( "deny" );
    attr->setTime( time );
    attr->setResponsible( "" );
    attr->start( pcm );
    
    buf << "Ok." << endl;
}

void Deny::doUsage( ostringstream &buf )
{
   buf 
        << 
        "Использование: \r\n"
        "deny <name>         - показать, кто и на какой срок поденаил чара\r\n"
        "deny <name> off     - снять deny\r\n"
        "deny <name> <time>  - запретить доступ на время <time>"
        << endl;
}

/*----------------------------------------------------------------------------
 * XMLAttributeDeny 
 *---------------------------------------------------------------------------*/
XMLAttributeDeny::XMLAttributeDeny( ) 
{
}

void XMLAttributeDeny::start( PCMemoryInterface *pcm ) const
{
    ostringstream buf;
    
    buf << responsible << " denies access for " << pcm->getName( ) << ", " << getTimeString( false ) << endl;
    wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, buf.str( ).c_str( ) );

    if (pcm->isOnline( )) {
        ostringstream buf;
        PCharacter *ch = pcm->getPlayer( );

        buf << "Доступ к этому миру тебе запрещен " << getTimeString( true ) << "." << endl;
        ch->send_to( buf );
        ch->getAttributes( ).getAttr<XMLStringAttribute>( "quit_flags" )->setValue( "quiet forced" );
        interpret_raw( ch, "quit", "" );
    }
    else
        PCharacterManager::saveMemory( pcm );
    
}

void XMLAttributeDeny::end( PCMemoryInterface *pcm ) const
{
    std::basic_ostringstream<char> buf;
    
    buf << "Время deny для " << pcm->getName( ) << " истекло. *shiver*" << endl; 
    wiznet( WIZ_PENALTIES, 0, 0 , buf.str( ).c_str( ) );
}

        

