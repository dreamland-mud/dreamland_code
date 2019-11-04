/* $Id: marry.cpp,v 1.1.2.7.6.2 2007/09/21 21:24:08 rufina Exp $
 *
 * ruffina, 2003
 */

#include "marry.h"
#include "marriageexception.h"
#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "infonet.h"
#include "messengers.h"
#include "room.h"
#include "class.h"

COMMAND(Marry, "marry")
{
    std::basic_ostringstream<char> buf;
    PCharacter *bride1, *bride2;
    DLString arguments = constArguments;
    DLString brideName1, brideName2;
    
    if (!ch->is_immortal( )) {
        ch->send_to( "Это не для тебя.\r\n" );
        return;
    }

    if (arguments.empty( )) {
        ch->send_to( "И кого женить будем?\r\n" );
        return;
    }
    
    brideName1 = arguments.getOneArgument( );
    brideName1.upperFirstCharacter( );
    
    if (arguments.empty( )) {
        ch->send_to( "Для это церемонии не хватает одного компонента.\r\n" );
        return;
    }

    brideName2 = arguments.getOneArgument( );
    brideName2.upperFirstCharacter( );

    if (brideName2 == brideName1) {
        ch->send_to( "Это еще как?\r\n" );
        return;
    }

    if (ch->getName( ) == brideName1 || ch->getName( ) == brideName2) {
        ch->send_to( "Попроси кого-то помочь тебе.\r\n" );
        return;
    }
    
    try {
        bride1 = checkBride( ch, brideName1 );
        bride2 = checkBride( ch, brideName2 );
        
    } catch (MarriageException e) {
        ch->send_to( e.what( ) );
        return;
    }

    bride1->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->spouse.setValue( brideName2 );
    bride1->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->wife.setValue( false );
    bride2->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->spouse.setValue( brideName1 );
    bride2->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" )->wife.setValue( true );

    buf << "Ты объявляешь " << brideName1 << " и " << brideName2 << " мужем и женой!" << endl;
    ch->send_to( buf );

    buf.str( "" );
    buf << ch->getName( ) << " объявляет вас мужем и женой!" << endl;
    bride1->send_to( buf );
    bride2->send_to( buf );

    buf.str( "" );
    buf << ch->getName( ) << " объявляет " << brideName1 << " и " << brideName2 << " мужем и женой!" << endl;
    
    for (Character *wch = ch->in_room->people; wch; wch = wch->next_in_room) {
        if (!wch->is_npc( ) && wch != ch && wch != bride1 && wch != bride2)
            wch->send_to( buf );
    }

    bride1->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->lovers.put( brideName2 );
    bride2->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->lovers.put( brideName1 );

    buf.str( "" );
    buf << "{CВеселый голос из $o2: {Y" 
        << brideName1 << "{W и {Y" << brideName2 << "{W теперь муж и жена!!!{x";
    infonet( buf.str( ).c_str( ), 0, 0 );

    send_discord_orb(brideName1 + " и " + brideName2 + " теперь муж и жена.");
}

PCharacter * Marry::checkBride( Character *ch, DLString name ) {
    std::basic_ostringstream<char> buf;
    PCMemoryInterface *pcm;
    PCharacter *pch;
    XMLAttributeMarriage::Pointer attr;
    
    pcm = PCharacterManager::find( name ); 
    
    if (!pcm) {
        buf << "Игрок " << name << " не найден." << endl;
        throw MarriageException( buf.str( ) );
    }

    pch = dynamic_cast<PCharacter *>( pcm );

    if (!pch) {
        buf << name << " не присутствует в мире." << endl;
        throw MarriageException( buf.str( ) );
    }

    attr = pch->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );

    if (attr && !attr->spouse.getValue( ).empty( )) {
        buf << "Но " << name << " уже связан(а) брачными путами!" << endl;
        throw MarriageException( buf.str( ) );
    }
    
    if (ch->in_room != pch->in_room) {
        buf << "Жертва " << name << " находится слишком далеко от тебя." << endl;
        throw MarriageException( buf.str( ) );
    }
    
    return pch;
}

