/* $Id: vocalchords.cpp,v 1.1.2.1 2007/05/02 03:05:09 rufina Exp $
 *
 * ruffina, 2007
 */
#include "vocalchords.h"

XMLAttributeVocalChords::XMLAttributeVocalChords( ) 
                             : maxIC( 100 ), coefIC( 1 ), 
                               maxOOC( 100 ), coefOOC( 1 )
{
}

XMLAttributeVocalChords::~XMLAttributeVocalChords( ) 
{
}

bool XMLAttributeVocalChords::canSpeak( Character *speaker, GlobalChannel *chan )
{
    int cost = chan->manaPercent;
    XMLInteger &now;
    
    if (chan->isIC) {
	cost *= coefIC;
	now = nowIC;
    }
    else {
	cost *= coefOOC;
	now = nowOOC;
    }
    
    if (now < cost) {
	speaker->println( msgNoChords.getValue( ).c_str( ) );
	return false;
    }

    now = now - cost; 
    return true;
}


