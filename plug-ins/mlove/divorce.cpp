/* $Id: divorce.cpp,v 1.1.2.5.10.1 2007/06/26 07:18:03 rufina Exp $
 *
 * ruffina, 2003
 */

#include "divorce.h"
#include "marriageexception.h"
#include "xmlattributemarriage.h"
#include "xmlattributelovers.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "class.h"

COMMAND(Divorce, "divorce")
{
    std::basic_ostringstream<char> buf;
    PCharacter *bride1, *bride2;
    DLString arguments = constArguments;
    DLString brideName1, brideName2;
    
    if (!ch->is_immortal( )) {
        ch->pecho("Это не для тебя.");
        return;
    }

    if (arguments.empty( )) {
        ch->pecho("И кого разводить будем?");
        return;
    }
    
    brideName1 = arguments.getOneArgument( );
    brideName1.upperFirstCharacter( );
   
    if (arguments.empty( )) { 
        try {
            bride1 = checkBride( ch, brideName1 );
            divorceWidow( bride1 );        
            buf << brideName1 << " - свободный человек." << endl;
            ch->send_to( buf );

        } catch (MarriageException e) {
            ch->send_to( e.what( ) );
            return;
        }
        return;
    }

    brideName2 = arguments.getOneArgument( );
    brideName2.upperFirstCharacter( );

    if (brideName2 == brideName1) {
        ch->pecho("Это еще как?");
        return;
    }

    if (ch->getName( ) == brideName1 || ch->getName( ) == brideName2) {
        ch->pecho("Попроси кого-то помочь тебе.");
        return;
    }

    try {
        bride1 = checkBride( ch, brideName1 );
        bride2 = checkBride( ch, brideName2 );

        divorce( bride1, brideName2 );
        divorce( bride2, brideName1 );
        
    } catch (MarriageException e ) {
        ch->send_to( e.what( ) );
        return;
    }

    buf << brideName1 << " и " << brideName2 << " освободились." << endl;
    ch->send_to( buf );
}

void Divorce::divorce( PCharacter *pch, DLString name ) {
    std::basic_ostringstream<char> buf;
    XMLAttributeMarriage::Pointer attr;
    
    attr = pch->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" );

    if (attr->spouse.getValue( ) != name) {
        buf << name << " и " << pch->getName( ) << " не женаты." << endl;
        throw MarriageException( buf.str( ) );
    }
    
    attr->history.push_back( XMLString( attr->spouse ));
    attr->spouse.setValue( "" );
    buf << "Ты чувствуешь себя свободным человеком." << endl;
    pch->send_to( buf );
}

void Divorce::divorceWidow( PCharacter *pch ) {
    std::basic_ostringstream<char> buf;
    XMLAttributeMarriage::Pointer attr;
    
    attr = pch->getAttributes( ).getAttr<XMLAttributeMarriage>( "marriage" );

    if (PCharacterManager::find( attr->spouse.getValue( ) )) {
        buf << pch->getName( ) << " еще не овдовел(а)." << endl;
        throw MarriageException( buf.str( ) );
    }

    attr->history.push_back( XMLString( attr->spouse ) );
    attr->spouse.setValue( "" );
    buf << "Ты чувствуешь себя свободным человеком." << endl;
    pch->send_to( buf );
}
    
PCharacter * Divorce::checkBride( Character *ch, DLString name ) {
    std::basic_ostringstream<char> buf;
    PCMemoryInterface *pcm;
    PCharacter *pch;
    XMLAttributeMarriage::Pointer attr;
    
    if ((pcm = PCharacterManager::find( name )) == NULL) {
        buf << "Игрок " << name << " не найден." << endl;
        throw MarriageException( buf.str( ) );
    }

    if ((pch = dynamic_cast<PCharacter *>( pcm )) == NULL) {
        buf << name << " не присутствует в мире." << endl;
        throw MarriageException( buf.str( ) );
    }

    attr = pch->getAttributes( ).findAttr<XMLAttributeMarriage>( "marriage" );

    if (!attr || attr->spouse.getValue( ).empty( )) {
        buf << "Но " << name << " не связан(а) брачными путами!" << endl;
        throw MarriageException( buf.str( ) );
    }
    
    if (ch->in_room != pch->in_room) {
        buf << "Жертва " << name << " находится слишком далеко от тебя." << endl;
        throw MarriageException( buf.str( ) );
    }
    
    return pch;
}

