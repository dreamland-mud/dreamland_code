/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          nochannel.cpp  -  description
                             -------------------
    begin                : Thu Sep 27 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "nochannel.h"

#include "logstream.h"
#include "date.h"
#include "admincommand.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "xmlattribute.h"
#include "dreamland.h"
#include "wiznet.h"
#include "def.h"

/*-------------------------------------------------------------------------
 * 'nochannel' command
 *------------------------------------------------------------------------*/
CMDADM( nochannel )
{
    
    try {
        XMLAttributes* attributes;
        XMLAttributeNoChannel::Pointer attr;
        PCMemoryInterface* pci;
        int level;
        DLString arguments = constArguments;
        DLString name = arguments.getOneArgument( );

        if( name.empty( ) ) {
            ch->send_to( "Nochannel кому?\n\r" );
            return;
        }

        pci = PCharacterManager::find( name );
        
        if (!pci) {
            ch->send_to( "Жертва не найдена. Укажите имя правильно и полностью.\n\r" );
            return;
        }

        name = pci->getName( );
        level = pci->get_trust( );
        attributes = &pci->getAttributes( );
                    
        arguments.stripWhiteSpace( );
            
        if( arguments == "off" ) {

            attr = attributes->findAttr<XMLAttributeNoChannel>( "nochannel" );
            
            if (attr) {
                attr->end( pci );
                attributes->eraseAttribute( "nochannel" );
                PCharacterManager::saveMemory( pci );

                ch->send_to( "NOCHANNEL снят.\n\r" );
                wiznet( WIZ_PENALTIES, WIZ_SECURE, 0 , 
                        "%^C1 восстанавливает возможность общаться для %s.",
                        ch, pci->getName( ).c_str( ) );
            }
            else {
                ch->send_to( "Не получилось.\n\r" );
            }
        }
        else if( arguments.empty( ) )
        {
            attr = attributes->findAttr<XMLAttributeNoChannel>( "nochannel" );

            if (attr) 
                ch->printf( "%s\r\n", attr->getUntilString( false ).c_str( ) );
            else
                ch->send_to( "none\n\r" );
        }
        else
        {
            std::basic_ostringstream<char> ostr;
            const DLString forever = "forever";
            int second = ( arguments == forever ) ? -1 : Date::getSecondFromString( arguments );
            
            attr = attributes->getAttr<XMLAttributeNoChannel>( "nochannel" );
            attr->setTime( second );
            attr->start( pci );
            PCharacterManager::saveMemory(pci);

            ch->send_to( "NOCHANNEL установлен.\n\r" );
            
            wiznet( WIZ_PENALTIES, WIZ_SECURE, 0 , 
                    "%^C1 отбирает у %s возможность общаться.",
                    ch, pci->getName( ).c_str( ) );
        }
    }
    catch( const ExceptionBadDateString& ex )
    {
        ch->printf( "%s\r\n", ex.what( ) );
    }
}

/*-------------------------------------------------------------------------
 * XMLAttributeNoChannel
 *------------------------------------------------------------------------*/
XMLAttributeNoChannel::XMLAttributeNoChannel( )
{
}
XMLAttributeNoChannel::~XMLAttributeNoChannel( )
{
}

void XMLAttributeNoChannel::start( PCMemoryInterface *pcm ) const
{
    PCharacter *pch;

    if ((pch = dynamic_cast<PCharacter *>( pcm ))) {
        std::basic_ostringstream<char> ostr;

        ostr << "Боги лишили тебя возможности общаться " << getUntilString( true ) << "." << std::endl;
        pch->send_to( ostr );
    }
}

void XMLAttributeNoChannel::end( PCMemoryInterface *pcm ) const
{
    PCharacter *pch;

    if ((pch = dynamic_cast<PCharacter *>( pcm ))) 
        pch->send_to( "Боги вернули тебе возможность общаться.\n\r" );
}

