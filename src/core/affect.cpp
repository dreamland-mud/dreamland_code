/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          affect.cpp  -  description
                             -------------------
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <algorithm>
#include "logstream.h"
#include "affect.h"
#include "skillgroup.h"
#include "liquid.h"
#include "wearlocation.h"
#include "pcharactermanager.h"
#include "npcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "room.h"
#include "merc.h"
#include "def.h"

Affect::Affect()
        : location(0, NULL), extracted(false)
{
}

Affect::~Affect( )
{
}



void Affect::extract() 
{
    type.assign(-1);
    level = duration = modifier = 0;
    bitvector.clear();
    location.setTable(0);
    location = 0;
    global.clear();
    global.setRegistry(0);

    extracted = true;
}


AffectSource::AffectSource(Character *ch) 
            : AffectSource()
{
    if (ch->is_npc()) {
        type = AFFSRC_MOB;
        ownerVnum = ch->getNPC()->pIndexData->vnum;
    } else {
        type = AFFSRC_PLAYER;
        ownerName = ch->getPC()->getName();
    }

    ownerID = ch->getID();
}

AffectSource::AffectSource(Object *obj) 
            : AffectSource()
{
    type = AFFSRC_ITEM;
    ownerVnum = obj->pIndexData->vnum;
    ownerID = obj->getID(); 
}

AffectSource::AffectSource(Room *room) 
            : AffectSource()
{
    type = AFFSRC_ROOM;
    ownerVnum = room->vnum;
    ownerID = room->getID();
}

AffectSource::AffectSource() 
    : type(AFFSRC_NONE, &affect_source_table)
{
    
}

AffectSource::~AffectSource() 
{
    
}

bool AffectSourceList::contains(const AffectSource &src) const
{
    for (auto &s: *this)    
        if (s.type == src.type && s.ownerID.getValue() == src.ownerID.getValue())
            return true;

    return false;
}

void AffectSourceList::remove(const AffectSource &toRemove)
{
    auto source_equals = [&](const AffectSource &source) -> bool {
        return toRemove.type == source.type && toRemove.ownerID.getValue() == source.ownerID.getValue();
    };
    remove_if(source_equals);
}

void AffectSourceList::add(Character *ch) 
{
    AffectSource src(ch);
    if (!contains(src))
        this->push_back(src);
}

void AffectSourceList::add(Object *obj) 
{
    AffectSource src(obj);
    if (!contains(src))
        this->push_back(src);
}

void AffectSourceList::add(Room *room) 
{
    AffectSource src(room);
    if (!contains(src))
        this->push_back(src);
}

Character* AffectSourceList::getOwner() const
{
    Character *owner = 0;

    for (auto &s: *this) {
        if (s.type == AFFSRC_PLAYER) {
            owner = PCharacterManager::findPlayer(s.ownerName);
            if (owner)
                break;
        }

        if (s.type == AFFSRC_MOB) {
            owner = NPCharacterManager::find(s.ownerID.getValue());
            if (owner)
                break;
        }
    }

    return owner;
}
