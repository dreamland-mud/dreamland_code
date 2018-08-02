/* $Id$
 *
 * ruffina, 2004
 */
/*
 * TODO
 */

/*------------------------------------------------------------------
 * ShopPrice
 *-----------------------------------------------------------------*/
ShopPrice::ShopPrice( ShopArticle::Pointer article )
{
    this->article = article;
    this->silver = calculate( );
}

int ShopPrice::calculate( )
{
    if (IS_OBJ_STAT( article->obj, ITEM_NOSELL )) 
	return 0;
    
    return article->obj->cost;
}

int ShopPrice::toSilver( Character * ) const
{
    return silver;
}

/*------------------------------------------------------------------
 * ShopBuyPrice
 *-----------------------------------------------------------------*/
int ShopBuyPrice::calculate( )
{
    int cost = ShopPrice::calculate( );

    if (!cost)
	return 0;
	
    cost *= article->trader->profitBuy  / 100;

    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND) {
	if( !obj->value[1] ) 
	    cost /= 4;
	else 
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}

int ShopBuyPrice::toSilver( Character * ) const
{
    return silver * quantity;
}

int ShopBuyPrice::getSingleCost( ) const
{
    return silver;
}

int ShopBuyPrice::haggle( Character *client ) const
{
    int cost = toSilver( client );
    int roll = number_percent( );

    if (!IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ) && roll < gsn_haggle->getEffective( client )) {
	cost -= obj->cost / 2 * roll / 100;

	act( "Ты торгуешься с $C5.", client, 0, article->trader->getKeeper( ), TO_CHAR );
	gsn_haggle->improve( client, true );
    }

    return cost;
}



void ShopBuyPrice::taxes( int cost ) const
{
    cost += keeper->silver;
    /* 'number' процентов от цены и кассы - в банк */
    dreamland->putToMerchantBank( cost * number / 100 );
    /* положить доход в кассу и вычесть то, что ушло в банк */
    keeper->silver = cost * number - ( cost * number / 100 ) * 100;
}

/*------------------------------------------------------------------
 * ShopSellPrice
 *-----------------------------------------------------------------*/
int ShopSellPrice::calculate( )
{
    int cost = ShopPrice::calculate( );
    Object *obj = article->obj;

    if (!cost)
	return 0;

    if (!article->trader->buys.isSet( obj->item_type ))
	return 0;

    cost *= article->trader->profitSell / 100;
    
    if (!IS_OBJ_STAT( obj, ITEM_SELL_EXTRACT ))
	cost >>= article->trader->countSameObjects( obj );

    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND) {
	if (!obj->value[1]) 
	    cost /= 4;
	else 
	    cost = cost * obj->value[2] / obj->value[1];
    }

    return cost;
}

int ShopSellPrice::haggle( Character *client ) const
{
    int cost = toSilver( client );
    int ocost = cost;
    int roll = number_percent( );

    if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT) && roll < gsn_haggle->getEffective( client )) {
	roll = gsn_haggle->getEffective( ch ) + number_range(1, 20) - 10;
	cost += obj->cost / 2 * roll / 100;
	cost = min(cost, 95 * ocost / 100);

	act( "Ты торгуешься с $C5.", client, 0, article->trader->getKeeper( ), TO_CHAR );
	gsn_haggle->improve( ch, true );
    }

    return cost;
}

bool ShopSellPrice::canAfford( Character *ch ) const
{
    int cass = article->trader->getKeeper( )->silver;
    
    if (silver <= cass)
	return true;
    else if ((silver - cass) / 100 + 1 <= dreamland->getBalanceMerchantBank( ))
	return true;
    else
	return false;
}

void ShopSellPrice::taxes( int cost ) const
{
    if (cost <= keeper->silver)
	keeper->silver -= cost;
    else {
	cost -= keeper->silver;
	
	if ( !dreamland->getFromMerchantBank( cost / 100 + 1 ) )
	{
	    act_p("$c1 говорит тебе '{GУ меня нет денег.{x'",
	    keeper,0,ch,TO_VICT,POS_RESTING);
	    return;
	}

	keeper->silver = ( cost / 100 + 1 ) * 100 - cost;
    }
}

void ShopSellPrice::deduct( Character *ch ) const
{
    int cost = haggle(  );
    
    silver = min( ch->silver, cost );
    
    if (silver < cost) {
	gold = (cost - silver + 99) / 100;
	silver = cost - 100 * gold;
    }

    ch->gold -= gold;
    ch->silver -= silver;

    taxes( cost );
}

