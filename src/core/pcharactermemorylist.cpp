/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermemorylist.cpp  -  description
                             -------------------
    begin                : Mon May 14 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "pcharactermemorylist.h"
#include "pcharacter.h"

PCharacterMemoryList::PCharacterMemoryList( )
{
}

PCharacterMemoryList::~PCharacterMemoryList( )
{
    clear_delete( );
}
