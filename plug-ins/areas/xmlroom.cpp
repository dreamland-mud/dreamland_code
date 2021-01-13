/* $Id$
 *
 * ruffina, 2004
 */

#include "xmlroom.h"
#include "directions.h"
#include "room.h"
#include "clanreference.h"
#include "wearlocation.h"
#include "merc.h"
#include "mercdb.h"

CLAN(none);
LIQ(none);
LIQ(water);

/******************************************************
 * resets
 ******************************************************/
bool 
XMLResetList::nodeFromXML(const XMLNode::Pointer &child) 
{
    insert( end( ), RESET_DATA( ) );                
    RESET_DATA &r = back( );
    const DLString &t = child->getName( ); 
    
    r.arg1 = 0;
    r.arg2 = 0;
    r.arg3 = 0;
    r.arg4 = 0;

    r.flags.setBits(child->getAttribute("flags"));

    if (child->hasAttribute("rand"))
        r.rand.setValue(rand_table.value(child->getAttribute("rand")));

    if (child->hasAttribute("bestTier"))
        r.bestTier = child->getAttribute("bestTier").toInt();

    if(child->getType( ) != XMLNode::XML_LEAF) {
        return false;
    } else if(t == "mob") {
        r.command = 'M';
        r.arg1 = child->getAttribute("vnum").toInt( );    
        r.arg2 = child->getAttribute("limit").toInt( );    
        r.arg4 = child->getAttribute("maxHere").toInt( );    
    } else if(t == "drop") {
        r.command = 'O';
        r.arg1 = child->getAttribute("vnum").toInt( );
    } else if(t == "put") {
        r.command = 'P';
        NumberSet vnums(child->getAttribute("vnum"));
        r.arg1 = (*vnums.begin());
        r.arg2 = child->getAttribute("limit").toInt( );    // should be called 'max count'
        r.arg3 = child->getAttribute("in").toInt( );    
        r.arg4 = child->getAttribute("maxHere").toInt( ); // should be called 'min count'        
        r.vnums.insert(r.vnums.end(), vnums.begin(), vnums.end());
    } else if(t == "give") {
        r.command = 'G';
        r.arg1 = child->getAttribute("vnum").toInt( );
    } else if(t == "equip") {
        r.command = 'E';
        r.arg1 = child->getAttribute("vnum").toInt( );
        r.arg3 = wearlocationManager->lookup(child->getAttribute("to"));
    } else if(t == "random") {
        r.command = 'R';
        r.arg3 = child->getAttribute("minDoor").toInt( );
        r.arg2 = child->getAttribute("maxDoor").toInt( );
    } else {
        return false;
    }
    return true;
}

bool 
XMLResetList::toXML(XMLNode::Pointer &parent) const
{
    if (empty( ))
        return false;

    for (const_iterator r = begin( ); r != end( ); r++) {
        XMLNode::Pointer child(NEW);
        child->setType(XMLNode::XML_LEAF);

        if (r->flags.getValue() > 0)
            child->insertAttribute("flags", r->flags.names());
        if (r->rand.getValue() > 0)
            child->insertAttribute("rand", r->rand.name());
        if (r->bestTier > 0)
            child->insertAttribute("bestTier", r->bestTier);

        switch(r->command) {
        case 'M':
            child->setName("mob");
            child->insertAttribute("vnum", r->arg1);
            child->insertAttribute("limit", r->arg2);
            child->insertAttribute("maxHere", r->arg4);
            break;
        case 'O':
            child->setName("drop");
            child->insertAttribute("vnum", r->arg1);
            break;
        case 'P':
            child->setName("put");
            child->insertAttribute("vnum", r->arg1);
            child->insertAttribute("in", r->arg3);
            child->insertAttribute("limit", r->arg2);
            child->insertAttribute("maxHere", r->arg4);
            break;
        case 'G':
            child->setName("give");
            child->insertAttribute("vnum", r->arg1);
            break;
        case 'E':
            child->setName("equip");
            child->insertAttribute("vnum", r->arg1);
            child->insertAttribute("to", wearlocationManager->find(r->arg3)->getName( ));
            break;
        case 'R':
            child->setName("random");
            child->insertAttribute("minDoor", r->arg3);
            child->insertAttribute("maxDoor", r->arg2);
            break;
        default:
            continue;
        }
        
        parent->appendChild(child);
    }
    return true;
}


/******************************************************
 * XMLRoom
 ******************************************************/
XMLRoom::XMLRoom() : 
    flags(0, &room_flags), 
    sector(SECT_FOREST, &sector_table),
    exits(false),
    extraExits(false),
    extraDescr(false),
    properties( false )

{
}

void 
XMLRoom::init(RoomIndexData *room)
{
    name.setValue(room->name);
    description.setValue(room->description);
    flags.setValue(room->room_flags);
    sector.setValue(room->sector_type);

    if(room->clan != clan_none)
        clan.setValue(room->clan->getName( ));

    if(!room->guilds.empty( ))
        guilds.setValue(room->guilds.toString( ));
    
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pExit = room->exit[door];

        if (pExit)
            exits[dirs[pExit->orig_door].name].init(pExit);
    }

    for(auto &peexit: room->extra_exits)
        extraExits[peexit->keyword].init(peexit);

    EXTRA_DESCR_DATA *pEd;
    for (pEd = room->extra_descr; pEd; pEd = pEd->next) {
        extraDescr.push_back(XMLExtraDescr( ));
        extraDescr.back( ).keyword = pEd->keyword;
        extraDescr.back( ).setValue(pEd->description);
    }

    manaRate.setValue(room->mana_rate);
    healRate.setValue(room->heal_rate);

    if (room->liquid != liq_none && room->liquid != liq_water) 
        liquid.setValue(room->liquid->getName( ));

    for (auto &pReset: room->resets)
        resets.push_back(*pReset);

    if(!room->behavior.isEmpty( ))
        behavior.setNode(room->behavior->getFirstNode( ));

    for (Properties::const_iterator p = room->properties.begin( ); p != room->properties.end( ); p++)
        properties.insert( *p );
}

RoomIndexData *
XMLRoom::compat(int vnum)
{
    RoomIndexData *room;
    
    room = new RoomIndexData;

    room->vnum = vnum;
    room->name = str_dup(name.getValue( ).c_str( ));
    room->description = str_dup(description.getValue( ).c_str( ));
    room->room_flags = flags.getValue( );
    room->sector_type = sector.getValue( );
    room->mana_rate = manaRate.getValue( );
    room->heal_rate = healRate.getValue( );

    if(!clan.getValue( ).empty( ))
        room->clan.setName(clan.getValue( ));
    else
        room->clan = clan_none;

    if(!guilds.getValue( ).empty( ))
        room->guilds.fromString(guilds.getValue( ));
    
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        XMLMapBase<XMLExitDir>::iterator it = exits.find(dirs[door].name);
        EXIT_DATA *&pExit = room->exit[door];

        if(it != exits.end( )) {
            pExit = it->second.compat( );
            pExit->orig_door = door;
        } else
            pExit = NULL;
    }

    XMLMapBase<XMLExtraExit>::reverse_iterator eeit;
    for(eeit = extraExits.rbegin( ); eeit != extraExits.rend( ); eeit++) {
        EXTRA_EXIT_DATA *peexit = eeit->second.compat( );
        peexit->keyword = str_dup(eeit->first.c_str( ));
        room->extra_exits.push_front(peexit);
    }

    XMLListBase<XMLExtraDescr>::reverse_iterator edit;
    for(edit = extraDescr.rbegin( ); edit != extraDescr.rend( ); edit++) {
        EXTRA_DESCR_DATA *pEd = new_extra_descr( );
        pEd->keyword = str_dup(edit->keyword.c_str( ));
        pEd->description = str_dup(edit->getValue( ).c_str( ));
        pEd->next = room->extra_descr;
        room->extra_descr = pEd;
    }

    if(!liquid.getValue( ).empty( ))
        room->liquid.setName(liquid.getValue( ));
    else
        room->liquid = liq_none;

    if ((room->sector_type == SECT_WATER_NOSWIM || room->sector_type == SECT_WATER_SWIM) && room->liquid == liq_none)
        room->liquid = liq_water;

    XMLResetList::iterator rit;
    for(rit = resets.begin( ); rit != resets.end( ); rit++) {
        RESET_DATA *pReset = new reset_data();

        *pReset = *rit;

        switch(pReset->command) {
        case 'R':
            pReset->arg1 = vnum;
            break;
        case 'M':
        case 'O':
            pReset->arg3 = vnum;
        }

        room->resets.push_back(pReset);
    }

    if(behavior.getNode( )) {
        room->behavior.construct( );
        XMLNode::Pointer p = behavior.getNode( );
        room->behavior->appendChild(p);
    }

    for (XMLMapBase<XMLString>::iterator p = properties.begin( ); p != properties.end( ); p++)
        room->properties.insert( *p );
    
    return room;
}
