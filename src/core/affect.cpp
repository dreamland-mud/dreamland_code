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


#include "affect.h"
#include "pcharactermanager.h"
#include "pcharacter.h"


Affect::Affect( )
        :       where( 0 ), level( 0 ),
                duration( 0 ), location( 0 ), modifier( 0 ),
                bitvector( 0 )
{
}

Affect::~Affect( )
{
}

Affect * Affect::clone() const
{
    Affect *paf;

    paf = dallocate( Affect );
    paf->where    = where; 
    paf->type     = type; 
    paf->level    = level;
    paf->duration = duration;
    paf->location = location;
    paf->modifier = modifier;
    paf->bitvector= bitvector;
    paf->global.setRegistry(global.getRegistry());
    paf->global.set(global);

    return paf;

}

Character * Affect::getOwner( ) const
{
    return PCharacterManager::findPlayer( ownerName );
}

