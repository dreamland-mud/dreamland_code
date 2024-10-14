/* $Id: xmlattributereward.cpp,v 1.1.2.2.6.2 2010-09-01 21:20:44 rufina Exp $
 *  
 * ruffina, 2003
 */

#include <sstream>
#include <string.h>

#include "xmlattributereward.h"
#include "gqchannel.h"
#include "globalquestinfo.h"
#include "globalquestmanager.h"
#include "player_exp.h"
#include "class.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "act.h"

XMLReward::XMLReward( )
{
}

XMLReward::~XMLReward( )
{
}

bool XMLReward::isEmpty( ) const
{
    return !(gold.getValue( ) > 0 
            || qpoints.getValue( ) > 0
            || practice.getValue( ) > 0
            || restring.getValue( ) > 0
            || experience.getValue( ) > 0
            );
}

void XMLAttributeReward::addReward( const XMLReward &r ) 
{
    rewards.push_back( r );    
}

void XMLAttributeReward::reward( PCharacter *ch ) 
{
    XMLVectorBase<XMLReward>::iterator r;
    ostringstream buf;
    const char *offset = "            ";
    int c;

    for (r = rewards.begin( ); r!= rewards.end( ); ) {
        if (r->isEmpty( )) {
            rewards.erase( r );
            continue;
        }

        buf << r->reason << endl;
            
        c = r->gold;
        if (c > 0) {
            ch->gold += c;
            buf << fmt(0, "%s%s%4d %sзолот%s\r\n",
                     offset, GQChannel::BOLD, c, GQChannel::NORMAL,
                     GET_COUNT(c, "ую монету", "ые монеты", "ых монет") );
        }

        c = r->qpoints;
        if (c > 0) {
            ch->addQuestPoints(c);
            buf << fmt(0, "%s%s%4d %sквестов%s\r\n",
                     offset, GQChannel::BOLD, c, GQChannel::NORMAL,
                     GET_COUNT(c, "ую единицу", "ые единицы", "ых единиц") );
        }

        c = r->practice;
        if (c > 0) {
            ch->practice += c;
            buf << fmt(0, "%s%s%4d %sпрактик%s\r\n",
                      offset, GQChannel::BOLD, c, GQChannel::NORMAL,
                      GET_COUNT(c, "у", "и", "") );
        }
        
        c = r->experience;
        if (c > 0) {
            Player::gainExp(ch, c);
            buf << fmt(0, "%s%s%4d %sочк%s опыта\r\n",
                     offset, GQChannel::BOLD, c, GQChannel::NORMAL,
                     GET_COUNT(c, "о", "а", "ов"));
        }
        
        GQChannel::pecho( ch, buf );
        rewards.erase( r );
    }
}


void XMLAttributeRewardListenerPlugin::run( int oldState, int newState, Descriptor *d ) 
{
    XMLAttributeReward::Pointer attr;
    Character *ch = d->character;

    if (!ch)
        return;
    
    if (newState != CON_PLAYING) 
        return;
    
    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeReward>( "reward" );
    
    if (!attr)
        return;

    attr->reward( ch->getPC( ) );
}


