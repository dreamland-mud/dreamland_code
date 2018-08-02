/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributetrust.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "clanreference.h"

XMLAttributeTrust::XMLAttributeTrust( )
                    : all( false ), 
		      clansAllow( clanManager ), clansDeny( clanManager )
{
}

XMLAttributeTrust::~XMLAttributeTrust( )
{
}

bool XMLAttributeTrust::check( Character *ch ) const
{
    if (ch->is_npc( )) {
	if (ch->master)
	    return check( ch->master );
	else 
	    return all.getValue( );
    }

    if (all.getValue( )) 
	return !checkDeny( ch->getPC( ) );
    else 
	return checkAllow( ch->getPC( ) );
}

bool XMLAttributeTrust::checkDeny( PCharacter *ch ) const
{
    if (playersDeny.hasElement( ch->getName( ) ))
	return true;

    if (clansDeny.isSet( *(ch->getClan( )) ))
	return true;

    return false;
}

bool XMLAttributeTrust::checkAllow( PCharacter *ch ) const
{
    if (playersAllow.hasElement( ch->getName( ) ))
	return true;

    if (clansAllow.isSet( *(ch->getClan( )) ))
	return true;

    return false;
}

bool XMLAttributeTrust::parse( const DLString &constArguments, ostringstream &buf )
{
    DLString args = constArguments;
    DLString cmd = args.getOneArgument( );
    bool fAllow;
    PCMemoryInterface *pci;
    Clan *clan;
    
    if (cmd.empty( )) {
	buf << "Укажи одно из действий: {lRсписок, разрешить или запретить{lElist, allow или deny{lx.";
	return false;
    }
    
    if (cmd.strPrefix( "list" ) || cmd.strPrefix( "список" )) {
	if (all) {
	    buf << "разрешено всем";
	    
	    if (!clansDeny.empty( ))
		buf << endl << "   кроме членов клана " << clansDeny.toString( );

	    if (!playersDeny.empty( ))
		buf << endl << "   кроме персонажа " << playersDeny.toString( );
	}
	else { 
	    buf << "запрещено всем";

	    if (!clansAllow.empty( ))
		buf << endl << "   кроме членов клана " << clansAllow.toString( );

	    if (!playersAllow.empty( ))
		buf << endl << "   кроме персонажа " << playersAllow.toString( );
	}

	return true;
    }

    if (cmd.strPrefix( "allow" ) || cmd.strPrefix( "разрешить" ))
	fAllow = true;
    else if (cmd.strPrefix( "deny" ) || cmd.strPrefix( "запретить" ))
	fAllow = false;
    else {
	buf << "Укажи одно из действий: {lRсписок, разрешить или запретить{lElist, allow или deny{lx.";
	return false;
    }

    if (args.empty( )) {
	buf << "Кому именно ты хочешь " 
	    << (fAllow ? "разрешить" : "запретить") << " это делать?";
	return false;
    }
    
    if (args == "all" || args == "все" || args == "всем") {
	if (fAllow)
	    buf << "отныне разрешено всем (кроме тех, кому запрещено явно).";
	else
	    buf << "отныне запрещено всем (кроме тех, кому разрешено явно).";

	all = fAllow;
	return true;
    }

    pci = PCharacterManager::find( args );
    clan = clanManager->findExisting( args );

    if (pci) {
	if (fAllow) {
	    if (all)
		playersDeny.remove( pci->getName( ) );
	    else
		playersAllow.add( pci->getName( ) );
	    buf << "отныне разрешено персонажу " << pci->getName( ) << ".";
	}
	else {
	    if (all)
		playersDeny.add( pci->getName( ) );
	    else
		playersAllow.remove( pci->getName( ) );
	    buf << "отныне запрещено персонажу " << pci->getName( ) << ".";
	}
	return true;
    }

    if (clan) {
	if (fAllow) {
	    if (all)
		clansDeny.remove( *clan );
	    else
		clansAllow.set( *clan );
	    buf << "отныне разрешено членам клана " << clan->getShortName( ) << ".";
	}
	else {
	    if (all)
		clansDeny.set( *clan );
	    else
		clansAllow.remove( *clan );
	    buf << "отныне запрещено членам клана " << clan->getShortName( ) << ".";
	}
	return true; 
    }

    buf << "Клан или персонаж с таким именем не найдены.";
    return false;
}

