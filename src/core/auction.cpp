
#include "auction.h"
#include "object.h"
#include "character.h"

AUCTION_DATA * auction = new auction_data( );

auction_data::auction_data( )
                     : item( NULL ), seller( NULL ), buyer( NULL )
{
}

