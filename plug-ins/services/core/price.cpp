/* $Id$
 *
 * ruffina, 2004
 */
#include "price.h"

#include "pcharacter.h"
#include "npcharacter.h"

#include "dreamland.h"

/*----------------------------------------------------------------------
 * Price
 *---------------------------------------------------------------------*/
Price::~Price( )
{
}

/*----------------------------------------------------------------------
 * MoneyPrice
 *---------------------------------------------------------------------*/
const DLString MoneyPrice::CURRENCY_NAME = "ден|ьги|ег|ьгам|ег|ьгами|ьгах";

DLString MoneyPrice::toCurrency( ) const
{
    return CURRENCY_NAME;
}

DLString MoneyPrice::toString( Character *ch ) const
{
    int silver, gold;
    ostringstream buf;

    silver = toSilver( ch );
    gold = silver / 100;
    silver = silver % 100;

    if (gold > 0) 
	buf << gold << " золот" << GET_COUNT(gold, "ая монета", "ые монеты", "ых монет");
    
    if (silver > 0) {
	if (gold > 0)
	    buf << " и ";

	buf << silver << " серебрян" << GET_COUNT(silver, "ая монета", "ые монеты", "ых монет");
    }
    
    return buf.str( );
}

bool MoneyPrice::canAfford( Character *ch ) const
{
    return ch->gold * 100 + ch->silver >= toSilver( ch );
}

int MoneyPrice::haggle( Character *ch ) const
{
    return toSilver( ch );
}

void MoneyPrice::induct( Character *ch ) const
{
    int cost = toSilver( ch );

    ch->gold += cost / 100;
    ch->silver += cost % 100;
}

void MoneyPrice::deduct( Character *ch ) const
{
    int silver = 0, gold = 0;
    int cost = haggle( ch );
    
    silver = min( (int)ch->silver, cost );
    
    if (silver < cost) {
	gold = (cost - silver + 99) / 100;
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    taxes( cost );
}

void MoneyPrice::taxes( int cost ) const
{
     dreamland->putToMerchantBank( cost / 100 );
}

void MoneyPrice::toStream( Character *ch, ostringstream &buf ) const
{
    int cost = toSilver( ch );
    int silver, gold;

    gold = cost / 100;
    silver = cost % 100;

    if (silver == 0) {
	if (gold != 0)
	    buf << gold << " gold";
    }
    else
	buf << cost;
}

/*----------------------------------------------------------------------
 * CoinPrice 
 *---------------------------------------------------------------------*/
int CoinPrice::toSilver( Character *ch ) const
{
    return gold.getValue( ) * 100 + silver.getValue( );
}

/*----------------------------------------------------------------------
 * LevelPrice 
 *---------------------------------------------------------------------*/
LevelPrice::LevelPrice( ) : power( 1 )
{
}

int LevelPrice::getLevel( Character *ch ) const
{
    return ch->getModifyLevel( );
}

int LevelPrice::toSilver( Character *ch ) const
{
    int level = getLevel( ch );
    int pow = power.getValue( );
    
    while (--pow > 0)
	level *= level;
    
    return level * coef.getValue( ) + bonus.getValue( );
}

/*----------------------------------------------------------------------
 * QuestPointPrice 
 *---------------------------------------------------------------------*/
const DLString QuestPointPrice::CURRENCY_NAME 
               = "квестов|ые|ых|ым|ые|ыми|ых единиц|ы||ам|ы|ами|ах";

DLString QuestPointPrice::toCurrency( ) const
{
    return CURRENCY_NAME;
}

DLString QuestPointPrice::toString( Character * ) const
{
    return DLString( questpoint.getValue( ) ) + " " + CURRENCY_NAME;
}

bool QuestPointPrice::canAfford( Character *ch ) const
{
    if (ch->is_npc( ))
	return false;
    else
	return ch->getPC( )->questpoints >= questpoint.getValue( );
}

void QuestPointPrice::induct( Character *ch ) const
{
    if (!ch->is_npc( ))
	ch->getPC( )->questpoints += questpoint.getValue( );
}

void QuestPointPrice::deduct( Character *ch ) const
{
    if (!ch->is_npc( ))
	ch->getPC( )->questpoints -= questpoint.getValue( );
}

void QuestPointPrice::toStream( Character *ch, ostringstream &buf ) const
{
    buf << questpoint << "qp";
}
