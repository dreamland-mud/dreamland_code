/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermemorylist.h  -  description
                             -------------------
    begin                : Mon May 14 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef PCHARACTERMEMORYLIST_H
#define PCHARACTERMEMORYLIST_H

#include "dlmap.h"
#include "string_utils.h"

class PCMemoryInterface;

struct MapCompareIgnoreCase : public std::binary_function<DLString, DLString, bool>
{
        inline result_type operator( ) ( const first_argument_type& str1, const second_argument_type& str2 ) const
        {
                return String::lessCase(str1, str2);
        }
};


/**
 * @author Igor S. Petrenko
 */
class PCharacterMemoryList : public DLMap<DLString, PCMemoryInterface, MapCompareIgnoreCase>
{
public: 
        PCharacterMemoryList( );
        virtual ~PCharacterMemoryList( );
};

#endif
