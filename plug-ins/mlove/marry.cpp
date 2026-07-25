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
#include "act.h"
#include "l10n.h"

COMMAND(Marry, "marry")
{
    PCharacter *bride1, *bride2;
    DLString arguments = constArguments;
    DLString brideName1, brideName2;
    
    if (!ch->is_immortal( )) {
        ch->pecho(_("Это не для тебя."));
        return;
    }

    if (arguments.empty( )) {
        ch->pecho(_("И кого женить будем?"));
        return;
    }
    
    brideName1 = arguments.getOneArgument( );
    brideName1.upperFirstCharacter( );
    
    if (arguments.empty( )) {
        ch->pecho(_("Для это церемонии не хватает одного компонента."));
        return;
    }

    brideName2 = arguments.getOneArgument( );
    brideName2.upperFirstCharacter( );

    if (brideName2 == brideName1) {
        ch->pecho(_("Это еще как?"));
        return;
    }

    if (ch->getPC()->getName( ) == brideName1 || ch->getPC()->getName( ) == brideName2) {
        ch->pecho(_("Попроси кого-то помочь тебе."));
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

    ch->send_to(fmt(ch, _("Ты объявляешь %1$s и %2$s мужем и женой!\n"),
        brideName1.c_str( ), brideName2.c_str( )));

    // The two brides and every onlooker see the celebrant's name (%C1, declined
    // per viewer); the wedding names are author-typed strings shown as-is (%s).
    bride1->send_to(fmt(bride1, _("%1$C1 объявляет вас мужем и женой!\n"), ch));
    bride2->send_to(fmt(bride2, _("%1$C1 объявляет вас мужем и женой!\n"), ch));

    for (Character *wch = ch->in_room->people; wch; wch = wch->next_in_room) {
        if (!wch->is_npc( ) && wch != ch && wch != bride1 && wch != bride2)
            wch->send_to(fmt(wch, _("%1$C1 объявляет %2$s и %3$s мужем и женой!\n"),
                ch, brideName1.c_str( ), brideName2.c_str( )));
    }

    bride1->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->lovers.put( brideName2 );
    bride2->getAttributes( ).getAttr<XMLAttributeLovers>( "lovers" )->lovers.put( brideName1 );

    // Info channel per viewer; Discord is EN-only (names are author content).
    infonet((Character*)0, 0,
        _("{CВеселый голос из $o2: {Y%1$s{W и {Y%2$s{W теперь муж и жена!!!{x"),
        brideName1.c_str( ), brideName2.c_str( ));

    send_discord_orb(":heart: " + brideName1 + " and " + brideName2 + " are now husband and wife.");
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

