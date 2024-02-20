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
#include "merc.h"
#include "def.h"

/*----------------------------------------------------------------------------
 * 'deny' command
 *---------------------------------------------------------------------------*/
COMMAND(Deny, "deny")
{
    PCMemoryInterface *pcm;
    DLString name;
    DLString argument = constArguments;
    
    if (argument.empty( )) {
        doUsage( ch );
        return;
    }
    
    name = argument.getOneArgument( );
    pcm = PCharacterManager::find( name );
    
    if (!pcm) {
        ch->pecho( "Victim '%s' not found, misspelled name?", name.c_str( ) );
        return;
    }
    
    try {
        if (argument.empty( )) 
            doShow( ch, pcm );
        else if (argument == "off") 
            doRemove( ch, pcm );
        else
            doPlace( ch, pcm, argument );
            
    } catch (Exception e) {
        ch->pecho( "%s", e.what( ) );
    }
}

void Deny::doShow( Character *ch, PCMemoryInterface *pcm )
{
    XMLAttributeDeny::Pointer attr;
    
    attr = pcm->getAttributes( ).findAttr<XMLAttributeDeny>( "deny" );
    
    if (attr) 
        ch->pecho( "Access for {W%s{x is denied {W%s{x by {W%s{x.",
                    pcm->getName( ).c_str( ),
                    attr->getUntilString( false ).c_str( ),
                    attr->getResponsible( ).c_str( ) );
    else
        ch->pecho( "Access for {W%s{x is NOT denied.", pcm->getName( ).c_str( ) );
}

void Deny::doRemove( Character *ch, PCMemoryInterface *pcm )
{
    XMLAttributeDeny::Pointer attr;
    
    attr = pcm->getAttributes( ).findAttr<XMLAttributeDeny>( "deny" );

    if (attr) {
        attr->end( pcm );
        pcm->getAttributes( ).eraseAttribute( "deny" );
        PCharacterManager::saveMemory( pcm );
        
        ch->pecho("Ok.");
    }
    else
        ch->pecho( "Access for {W%s{x is NOT denied.", pcm->getName( ).c_str( ) );
}

void Deny::doPlace( Character *ch, PCMemoryInterface *pcm, const DLString & argument )
{
    XMLAttributeDeny::Pointer attr;
    int time;
    
    if (ch->get_trust( ) < pcm->get_trust( )) {
        ch->pecho("Фигушки.");
        return;
    }
    
    if (argument == "forever")
        time = -1;
    else
        time = Date::getSecondFromString( argument );
    
    attr = pcm->getAttributes( ).getAttr<XMLAttributeDeny>( "deny" );
    attr->setTime( time );
    attr->setResponsible( ch->getName( ) );
    attr->start( pcm );
    
    ch->pecho("Ok.");
}

void Deny::doUsage( Character *ch )
{
    ch->pecho( 
        "Использование: \r\n"
        "deny <name>         - показать, кто и на какой срок поденаил чара\r\n"
        "deny <name> off     - снять deny\r\n"
        "deny <name> <time>  - запретить доступ на время <time>" );
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

        

