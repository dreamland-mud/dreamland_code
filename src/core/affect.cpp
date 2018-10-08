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
        :         where( 0 ), level( 0 ),
                duration( 0 ), location( 0 ), modifier( 0 ),
                bitvector( 0 )
{
}

Affect::~Affect( )
{
}

/* find an effect in an affect list */
Affect* Affect::affect_find( int sn )
{
        for( Affect* paf_find = this;
                paf_find != 0;
                paf_find = paf_find->next )
        {
                if( paf_find->type == sn )
                {
                        return paf_find;
                }
        }
        return 0;
}

Character * Affect::getOwner( ) const
{
    return PCharacterManager::findPlayer( ownerName );
}

