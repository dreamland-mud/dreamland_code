/* $Id$
 *
 * ruffina, 2004
 */
#include "remortbonus.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "act.h"
#include "merc.h"
#include "def.h"

/*
 * RemortBonus
 */
RemortBonus::RemortBonus( )
              : gender( 0, &sex_table )
{
}

bool RemortBonus::visible( Character *client ) const 
{
    return !client->is_npc( )
           && bonusMaximum( client->getPC( ) ) > 0 && price->canAfford( client );
}

bool RemortBonus::available( Character *client, NPCharacter *keeper ) const
{
    if (client->is_npc( ))
        return false;

    if (bonusMaximum( client->getPC( ) ) <= 0) {
        tell_raw( client, keeper, "Это тебе ни к чему." );
        return false;
    }

    return true;
}

bool RemortBonus::purchase( Character *client, NPCharacter *keeper, const DLString &, int quantity )
{
    if (client->is_npc( ))
        return false;

    if (!price->canAfford( client )) { /* xxx quantity */
        tell_raw( client, keeper, "Это тебе не по карману." );
        return false;
    }
    
    bonusBuy( client->getPC( ) );
    price->deduct( client );
    
    client->pecho( "%^C1 вручает тебе %N4 в обмен на %N4.",
                   keeper, getShortDescr( ).c_str( ), price->toString( client ).c_str( ) );
    client->recho( "%^C1 выменивает что-то у %C2.", client, keeper );
    return true;
}

bool RemortBonus::sellable( Character *client )
{
    return !client->is_npc( ) && bonusBought( client->getPC( ) );    
}

bool RemortBonus::sell( Character *client, NPCharacter *keeper )
{
    if (client->is_npc( ))
        return false;

    bonusSell( client->getPC( ) );
    price->induct( client );

    client->pecho( "Ты возвращаешь %C3 %N4.", keeper, getShortDescr( ).c_str( ) );
    client->recho( "%^C1 возвращает %C3 %N4.", client, keeper, getShortDescr( ).c_str( ) );
    return true;
}

const DLString & RemortBonus::getGender( ) const
{
    static const DLString yourNeutral = "тво|е|его|ему|е|им|ем";
    static const DLString yourFemale  = "тво|я|ей|ей|ю|ей|ей";
    static const DLString yourMale    = "тво|й|его|ему|й|им|ем";

    switch (gender.getValue( )) {
    case SEX_NEUTRAL: return yourNeutral;
    case SEX_FEMALE:  return yourFemale;
    case SEX_MALE:    return yourMale;
    default:          return DLString::emptyString; 
    }
}

void RemortBonus::toStream( Character *client, ostringstream &buf ) const
{
    DLString n;
    
    n << getGender( );
    if (!n.empty( ))
        n << " ";
    n << getShortDescr( );

    buf << dlprintf( "     %-27s     {D(%d за ", 
                     n.ruscase( '1' ).c_str( ), getQuantity( ) );
    price->toStream( client, buf );
    buf << "){x" << endl;
}

bool RemortBonus::matches( const DLString &arg ) const
{
    if (arg.empty( ))
        return false;
        
    if (arg == getShortDescr( ).ruscase( '1' ) 
        || arg == getShortDescr( ).ruscase( '4' ))
        return true;

    return matchesAlias( arg );
}

bool RemortBonus::matchesAlias( const DLString &arg ) const
{
    for (XMLListBase<XMLString>::const_iterator a = aliases.begin( ); a != aliases.end( ); a++)
        if (arg == *a)
            return true;

    return false;
}

DLString RemortBonus::getShortDescr( ) const
{
    return shortDescr;
}


/*
 * IntegerRemortBonus
 */
void IntegerRemortBonus::bonusBuy( PCharacter *client ) const
{
    bonusField( client ) += getQuantity( );
}

void IntegerRemortBonus::bonusSell( PCharacter *client ) const
{
    bonusField( client ) -= getQuantity( );
}

bool IntegerRemortBonus::bonusBought( PCharacter *client ) const
{
    return bonusField( client ) > 0;
}

int IntegerRemortBonus::getQuantity( ) const
{
    return amount.getValue( );
}

/*
 * BooleanRemortBonus
 */
void BooleanRemortBonus::bonusBuy( PCharacter *client ) const
{
    bonusField( client ) = true;
}

void BooleanRemortBonus::bonusSell( PCharacter *client ) const
{
    bonusField( client ) = false;
}

bool BooleanRemortBonus::bonusBought( PCharacter *client ) const
{
    return bonusField( client );
}

int BooleanRemortBonus::bonusMaximum( PCharacter *ch ) const 
{
    return !bonusField( ch );
}

int BooleanRemortBonus::getQuantity( ) const
{
    return 1;
}

/*
 * AppliedRemortBonus
 */
void AppliedRemortBonus::bonusBuy( PCharacter *ch ) const
{
    bonusRemove( ch );
    IntegerRemortBonus::bonusBuy( ch );
    bonusApply( ch );
}

void AppliedRemortBonus::bonusSell( PCharacter *ch ) const
{
    bonusRemove( ch );
    IntegerRemortBonus::bonusSell( ch );
    bonusApply( ch );
}

