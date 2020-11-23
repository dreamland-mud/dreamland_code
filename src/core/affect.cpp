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
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

Affect::Affect()
        : location(0, NULL)
{
}

Affect::~Affect( )
{
}


Character * Affect::getOwner( ) const
{
    return PCharacterManager::findPlayer( ownerName );
}

