/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          nopost.cpp  -  description
                             -------------------
    begin                : Thu Oct 18 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/
/***************************************************************************
                          xmlattributenopost.cpp  -  description
                             -------------------
    begin                : Thu Oct 18 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "nopost.h"

#include "admincommand.h"
#include "logstream.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "xmlattribute.h"
#include "wiznet.h"
#include "def.h"

/*---------------------------------------------------------------------------
 * 'nopost' command
 *--------------------------------------------------------------------------*/
CMDADM( nopost )
{
    
    try {
        XMLAttributes* attributes;
        XMLAttributeNoPost::Pointer attr;
        PCMemoryInterface* pci;
        DLString arguments = constArguments;
        DLString name = arguments.getOneArgument( );

        if( name.empty( ) ) {
            ch->pecho("Nopost whom?");
            return;
        }

        pci = PCharacterManager::find( name );
        
        if (!pci) {
            ch->pecho("Жертва не найдена. Укажите имя правильно и полностью.");
            return;
        }

        name = pci->getName( );
        attributes = &pci->getAttributes( );
                    
        arguments.stripWhiteSpace( );
            
        if( arguments == "off" ) {
            std::basic_ostringstream<char> ostr;

            attr = attributes->findAttr<XMLAttributeNoPost>( "nopost" );
            
            if (attr) {
                attr->end( pci );
                attributes->eraseAttribute( "nopost" );
                PCharacterManager::saveMemory( pci );

                ch->pecho("NOPOST снят.");
                wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, 
                        "%^C1 restores notes to %s.", ch, pci->getName( ).c_str( ) );
            }
            else {
                ch->pecho("Не получилось.");
            }
        }
        else if( arguments.empty( ) )
        {
            attr = attributes->findAttr<XMLAttributeNoPost>( "nopost" );

            if (attr) 
                ch->printf( "%s\r\n", attr->getUntilString( false ).c_str( ) );
            else
                ch->pecho("none");
        }
        else
        {
            std::basic_ostringstream<char> ostr;
            const DLString forever = "forever";
            int second = ( arguments == forever ) ? -1 : Date::getSecondFromString( arguments );
            
            attr = attributes->getAttr<XMLAttributeNoPost>( "nopost" );
            attr->setTime( second );
            attr->start( pci );
            PCharacterManager::saveMemory(pci);

            ch->pecho("NOPOST установлен.");
            
            wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, 
                    "%^C1 revokes %s's notes.", ch, pci->getName( ).c_str( ) );
        }
    }
    catch( const ExceptionBadDateString& ex )
    {
        ch->printf( "%s\r\n", ex.what( ) );
    }
}

/*---------------------------------------------------------------------------
 *  XMLAttributeNoPost
 *--------------------------------------------------------------------------*/
XMLAttributeNoPost::XMLAttributeNoPost( )
{
}

void XMLAttributeNoPost::start( PCMemoryInterface *pcm ) const
{
    PCharacter *pch;

    if ((pch = dynamic_cast<PCharacter *>( pcm ))) {
        std::basic_ostringstream<char> ostr;
        
        ostr << "Боги отняли у тебя привилегию писать письма " << getUntilString( true ) << "." << std::endl;
        pch->send_to( ostr );
    }
}

void XMLAttributeNoPost::end( PCMemoryInterface *pcm ) const
{
    PCharacter *pch;

    if ((pch = dynamic_cast<PCharacter *>( pcm ))) 
        pch->pecho("Боги вернули тебе привилегию писать письма.");
}

