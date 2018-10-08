/* $Id$
 *
 * ruffina, 2004
 */
short get_wear_level( Character *ch, Object *obj );
/*
 * TODO
 */
void ShopArticle::purchase( PCharacter *client, NPCharacter *keeper, const DLString &args, int quantity )
{
    ShopBuyPrice::Pointer price( NEW, this, quantity );

    if (!price->canAfford( client )) {
        if (quantity > 1)
            tell_dim( client, keeper, "Ты не сможешь заплатить за столько." );
        else
            tell_dim( client, keeper, "У тебя нет нужной суммы, чтоб купить $o4.", obj );
        
        return;
    }

    if (client->carry_number + number * obj->getNumber( ) > client->canCarryNumber( )) {
        tell_dim( client, keeper, "Ты не можешь нести так много вещей." );
        return;
    }

    if (client->carry_weight + number * obj->getWeight( ) > client->canCarryWeight( )) {
        tell_dim( client, keeper, "Ты не можешь нести такую тяжесть." );
        return;
    }
    
    if (quantity > 1) {
        ch->recho( "%^C1 покупает %O4 [%d].", ch, obj, quantity );
        ch->pecho( "Ты покупаешь %O4[%d] за %N4.", 
                    obj, quantity, price->toString( client ).c_str( ) );
    }
    else {
        ch->recho( "%^C1 покупает %O4.", ch, obj );
        ch->pecho( "Ты покупаешь %O4 за %N4.", 
                    obj, price->toString( client ).c_str( ) );
    }

    price->deduct( client );
    mprog_sell( keeper, client, obj, price->getSingleCost( ), quantity );

    for (int count = 0; count < quantity; count++) {
        if (IS_SET( obj->extra_flags, ITEM_INVENTORY) 
                && (obj->pIndexData->limit < 0 || obj->pIndexData->limit > obj->pIndexData->count))
        {
            t_obj = create_object( obj->pIndexData, obj->level );
        }
        else {
            t_obj = obj;
            obj = obj->next_content;
            obj_from_char( t_obj );
        }

        if (t_obj->timer > 0 && !IS_OBJ_STAT( t_obj, ITEM_HAD_TIMER ))
            t_obj->timer = 0;
        
        REMOVE_BIT( t_obj->extra_flags, ITEM_HAD_TIMER );
        obj_to_char( t_obj, client );

        if (price->getSingleCost( ) < t_obj->cost)
            t_obj->cost = price->getSingleCost( );
    }
}

bool ShopArticle::available( PCharacter *client, NPCharacter *keeper ) const
{
    if (client->getRealLevel( ) < get_wear_level( client, obj )) {
        tell_dim( client, keeper, "Ты не сможешь использовать $o4.", obj );
        return false;
    }

    return true;
}

bool ShopArticle::visible( PCharacter *client, NPCharacter *keeper ) const
{
    if (obj->wear_loc != wear_none)
        return false;
        
    if (!keeper->can_see( obj ) || !client->can_see( obj ))
        return false;

    if (getBuyPrice( )->toSilver( ) <= 0)
        return false;

    return true;
}

int ShopArticle::getQuantity( ) const
{
    Object *t_obj;
    int count = 1;
    
    if (!IS_OBJ_STAT( obj, ITEM_INVENTORY ))
        for (t_obj = obj->next_content; t_obj; t_obj = t_obj->next_content) 
            if (t_obj->pIndexData == obj->pIndexData
                && !str_cmp( t_obj->getShortDescr( ), obj->getShortDescr( ) ))
            {
                count++;
            }
            else
                break;

    if (obj->pIndexData->limit > 0)
        return URANGE( 0, count, obj->pIndexData->limit - obj->pIndexData->count );
    else
        return count;
}

